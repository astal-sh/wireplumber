#include <wp/wp.h>

#include "endpoint-private.h"
#include "wp.h"

struct _AstalWpEndpoint {
    GObject parent_instance;

    guint id;
    gdouble volume;
    gboolean mute;
    gchar *description;
    AstalWpMediaClass type;
    gboolean is_default;
};

typedef struct {
    WpNode *node;
    WpPlugin *mixer;
    WpPlugin *defaults;

    gboolean is_default_node;
    AstalWpMediaClass media_class;

    gulong default_signal_handler_id;
    gulong mixer_signal_handler_id;

} AstalWpEndpointPrivate;

G_DEFINE_FINAL_TYPE_WITH_PRIVATE(AstalWpEndpoint, astal_wp_endpoint, G_TYPE_OBJECT);

G_DEFINE_ENUM_TYPE(AstalWpMediaClass, astal_wp_media_class,
                   G_DEFINE_ENUM_VALUE(ASTAL_WP_MEDIA_CLASS_AUDIO_MICROPHONE, "Audio/Source"),
                   G_DEFINE_ENUM_VALUE(ASTAL_WP_MEDIA_CLASS_AUDIO_SPEAKER, "Audio/Sink"),
                   G_DEFINE_ENUM_VALUE(ASTAL_WP_MEDIA_CLASS_AUDIO_RECORDER, "Stream/Input/Audio"),
                   G_DEFINE_ENUM_VALUE(ASTAL_WP_MEDIA_CLASS_AUDIO_STREAM, "Stream/Output/Audio"));

typedef enum {
    ASTAL_WP_ENDPOINT_PROP_ID = 1,
    ASTAL_WP_ENDPOINT_PROP_VOLUME,
    ASTAL_WP_ENDPOINT_PROP_MUTE,
    ASTAL_WP_ENDPOINT_PROP_DESCRIPTION,
    ASTAL_WP_ENDPOINT_PROP_MEDIA_CLASS,
    ASTAL_WP_ENDPOINT_PROP_DEFAULT,
    ASTAL_WP_ENDPOINT_N_PROPERTIES,
} AstalWpEndpointProperties;

typedef enum {
    ASTAL_WP_ENDPOINT_SIGNAL_CHANGED,
    ASTAL_WP_ENDPOINT_N_SIGNALS
} AstalWpEndpointSignals;

static guint astal_wp_endpoint_signals[ASTAL_WP_ENDPOINT_N_SIGNALS] = {
    0,
};
static GParamSpec *astal_wp_endpoint_properties[ASTAL_WP_ENDPOINT_N_PROPERTIES] = {
    NULL,
};

void astal_wp_endpoint_update_volume(AstalWpEndpoint *self) {
    AstalWpEndpointPrivate *priv = astal_wp_endpoint_get_instance_private(self);

    gdouble volume;
    gboolean mute;
    GVariant *variant = NULL;

    g_signal_emit_by_name(priv->mixer, "get-volume", self->id, &variant);

    if (variant == NULL) return;

    g_variant_lookup(variant, "volume", "d", &volume);
    g_variant_lookup(variant, "mute", "b", &mute);

    if (mute != self->mute) {
        self->mute = mute;
        g_object_notify(G_OBJECT(self), "mute");
    }

    if (volume != self->volume) {
        self->volume = volume;
        g_object_notify(G_OBJECT(self), "volume");
    }

    g_signal_emit_by_name(self, "changed");
}

void astal_wp_endpoint_set_volume(AstalWpEndpoint *self, gdouble volume) {
    AstalWpEndpointPrivate *priv = astal_wp_endpoint_get_instance_private(self);

    gboolean ret;
    GVariant *variant = g_variant_new_double(volume);
    g_signal_emit_by_name(priv->mixer, "set-volume", self->id, variant, &ret);
}

void astal_wp_endpoint_set_mute(AstalWpEndpoint *self, gboolean mute) {
    AstalWpEndpointPrivate *priv = astal_wp_endpoint_get_instance_private(self);

    gboolean ret;
    GVariant *variant = NULL;
    GVariantBuilder b = G_VARIANT_BUILDER_INIT(G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&b, "{sv}", "mute", g_variant_new_boolean(mute));
    variant = g_variant_builder_end(&b);

    g_signal_emit_by_name(priv->mixer, "set-volume", self->id, variant, &ret);

    g_variant_unref(variant);
}

AstalWpMediaClass astal_wp_endpoint_get_media_class(AstalWpEndpoint *self) { return self->type; }

guint astal_wp_endpoint_get_id(AstalWpEndpoint *self) { return self->id; }

gboolean astal_wp_endpoint_get_mute(AstalWpEndpoint *self) { return self->mute; }

gdouble astal_wp_endpoint_get_volume(AstalWpEndpoint *self) { return self->volume; }

const gchar *astal_wp_endpoint_get_description(AstalWpEndpoint *self) { return self->description; }

gboolean astal_wp_endpoint_get_is_default(AstalWpEndpoint *self) { return self->is_default; }

void astal_wp_endpoint_set_is_default(AstalWpEndpoint *self, gboolean is_default) {
    AstalWpEndpointPrivate *priv = astal_wp_endpoint_get_instance_private(self);

    if (!is_default) return;
    gboolean ret;
    const gchar *name =
        wp_pipewire_object_get_property(WP_PIPEWIRE_OBJECT(priv->node), "node.name");
    const gchar *media_class =
        wp_pipewire_object_get_property(WP_PIPEWIRE_OBJECT(priv->node), "media.class");
    g_signal_emit_by_name(priv->defaults, "set-default-configured-node-name", media_class, name,
                          &ret);
}

static void astal_wp_endpoint_get_property(GObject *object, guint property_id, GValue *value,
                                           GParamSpec *pspec) {
    AstalWpEndpoint *self = ASTAL_WP_ENDPOINT(object);

    switch (property_id) {
        case ASTAL_WP_ENDPOINT_PROP_ID:
            g_value_set_uint(value, self->id);
            break;
        case ASTAL_WP_ENDPOINT_PROP_MUTE:
            g_value_set_boolean(value, self->mute);
            break;
        case ASTAL_WP_ENDPOINT_PROP_VOLUME:
            g_value_set_double(value, self->volume);
            break;
        case ASTAL_WP_ENDPOINT_PROP_DESCRIPTION:
            g_value_set_string(value, self->description);
            break;
        case ASTAL_WP_ENDPOINT_PROP_MEDIA_CLASS:
            g_value_set_enum(value, self->type);
            break;
        case ASTAL_WP_ENDPOINT_PROP_DEFAULT:
            g_value_set_boolean(value, self->is_default);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void astal_wp_endpoint_set_property(GObject *object, guint property_id, const GValue *value,
                                           GParamSpec *pspec) {
    AstalWpEndpoint *self = ASTAL_WP_ENDPOINT(object);

    switch (property_id) {
        case ASTAL_WP_ENDPOINT_PROP_MUTE:
            astal_wp_endpoint_set_mute(self, g_value_get_boolean(value));
            break;
        case ASTAL_WP_ENDPOINT_PROP_VOLUME:
            astal_wp_endpoint_set_volume(self, g_value_get_double(value));
            break;
        case ASTAL_WP_ENDPOINT_PROP_DEFAULT:
            astal_wp_endpoint_set_is_default(self, g_value_get_boolean(value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void astal_wp_endpoint_update_properties(AstalWpEndpoint *self) {
    AstalWpEndpointPrivate *priv = astal_wp_endpoint_get_instance_private(self);
    if (priv->node == NULL) return;
    self->id = wp_proxy_get_bound_id(WP_PROXY(priv->node));

    astal_wp_endpoint_update_volume(self);

    const gchar *description =
        wp_pipewire_object_get_property(WP_PIPEWIRE_OBJECT(priv->node), "node.description");
    if (description == NULL) {
        description = wp_pipewire_object_get_property(WP_PIPEWIRE_OBJECT(priv->node), "node.nick");
    }
    if (description == NULL) {
        description = wp_pipewire_object_get_property(WP_PIPEWIRE_OBJECT(priv->node), "node.name");
    }
    if (description == NULL) {
        description = "unknown";
    }
    g_free(self->description);
    self->description = g_strdup(description);

    const gchar *type =
        wp_pipewire_object_get_property(WP_PIPEWIRE_OBJECT(priv->node), "media.class");
    const GEnumClass *enum_class = g_type_class_ref(ASTAL_WP_TYPE_MEDIA_CLASS);
    self->type = g_enum_get_value_by_nick(enum_class, type)->value;
    g_type_class_unref(enum_class);

    g_object_notify(G_OBJECT(self), "id");
    g_object_notify(G_OBJECT(self), "description");
    g_object_notify(G_OBJECT(self), "type");
    g_signal_emit_by_name(self, "changed");
}

static void astal_wp_endpoint_default_changed_as_default(AstalWpEndpoint *self) {
    AstalWpEndpointPrivate *priv = astal_wp_endpoint_get_instance_private(self);

    const GEnumClass *enum_class = g_type_class_ref(ASTAL_WP_TYPE_MEDIA_CLASS);
    const gchar *media_class = g_enum_get_value(enum_class, priv->media_class)->value_nick;
    guint defaultId;
    g_signal_emit_by_name(priv->defaults, "get-default-node", media_class, &defaultId);
    g_type_class_unref(enum_class);

    if (defaultId != self->id) {
        if (priv->node != NULL) g_object_unref(priv->node);
        AstalWpEndpoint *default_endpoint =
            astal_wp_wp_get_endpoint(astal_wp_wp_get_default(), defaultId);
        if (default_endpoint != NULL &&
            astal_wp_endpoint_get_media_class(default_endpoint) == priv->media_class) {
            AstalWpEndpointPrivate *default_endpoint_priv =
                astal_wp_endpoint_get_instance_private(default_endpoint);
            priv->node = g_object_ref(default_endpoint_priv->node);
            astal_wp_endpoint_update_properties(self);
        }
    }
}

static void astal_wp_endpoint_default_changed(AstalWpEndpoint *self) {
    AstalWpEndpointPrivate *priv = astal_wp_endpoint_get_instance_private(self);

    guint defaultId;
    const gchar *media_class =
        wp_pipewire_object_get_property(WP_PIPEWIRE_OBJECT(priv->node), "media.class");
    g_signal_emit_by_name(priv->defaults, "get-default-node", media_class, &defaultId);

    if (self->is_default && defaultId != self->id) {
        self->is_default = FALSE;
        g_object_notify(G_OBJECT(self), "is-default");
        g_signal_emit_by_name(self, "changed");
    } else if (!self->is_default && defaultId == self->id) {
        self->is_default = TRUE;
        g_object_notify(G_OBJECT(self), "is-default");
        g_signal_emit_by_name(self, "changed");
    }
}

static void astal_wp_endpoint_mixer_changed(AstalWpEndpoint *self, guint node_id) {
    if (self->id != node_id) return;
    astal_wp_endpoint_update_volume(self);
}

AstalWpEndpoint *astal_wp_endpoint_init_as_default(AstalWpEndpoint *self, WpPlugin *mixer,
                                                   WpPlugin *defaults, AstalWpMediaClass type) {
    AstalWpEndpointPrivate *priv = astal_wp_endpoint_get_instance_private(self);

    priv->mixer = g_object_ref(mixer);
    priv->defaults = g_object_ref(defaults);

    priv->media_class = type;
    priv->is_default_node = TRUE;
    self->is_default = TRUE;

    priv->default_signal_handler_id = g_signal_connect_swapped(
        priv->defaults, "changed", G_CALLBACK(astal_wp_endpoint_default_changed_as_default), self);
    priv->mixer_signal_handler_id = g_signal_connect_swapped(
        priv->mixer, "changed", G_CALLBACK(astal_wp_endpoint_mixer_changed), self);

    astal_wp_endpoint_default_changed_as_default(self);
    astal_wp_endpoint_update_properties(self);
    return self;
}

AstalWpEndpoint *astal_wp_endpoint_create(WpNode *node, WpPlugin *mixer, WpPlugin *defaults) {
    AstalWpEndpoint *self = g_object_new(ASTAL_WP_TYPE_ENDPOINT, NULL);
    AstalWpEndpointPrivate *priv = astal_wp_endpoint_get_instance_private(self);

    priv->mixer = g_object_ref(mixer);
    priv->defaults = g_object_ref(defaults);
    priv->node = g_object_ref(node);
    priv->is_default_node = FALSE;

    priv->default_signal_handler_id = g_signal_connect_swapped(
        priv->defaults, "changed", G_CALLBACK(astal_wp_endpoint_default_changed), self);
    priv->mixer_signal_handler_id = g_signal_connect_swapped(
        priv->mixer, "changed", G_CALLBACK(astal_wp_endpoint_mixer_changed), self);

    astal_wp_endpoint_update_properties(self);
    astal_wp_endpoint_default_changed(self);
    return self;
}

static void astal_wp_endpoint_init(AstalWpEndpoint *self) {
    AstalWpEndpointPrivate *priv = astal_wp_endpoint_get_instance_private(self);
    priv->node = NULL;
    priv->mixer = NULL;
    priv->defaults = NULL;

    self->volume = 0;
    self->mute = TRUE;
    self->description = NULL;
}

static void astal_wp_endpoint_dispose(GObject *object) {
    AstalWpEndpoint *self = ASTAL_WP_ENDPOINT(object);
    AstalWpEndpointPrivate *priv = astal_wp_endpoint_get_instance_private(self);

    g_signal_handler_disconnect(priv->defaults, priv->default_signal_handler_id);
    g_signal_handler_disconnect(priv->mixer, priv->mixer_signal_handler_id);

    g_print("dispose: id: %u, name: %s\n", self->id, self->description);

    g_clear_object(&priv->node);
    g_clear_object(&priv->mixer);
    g_clear_object(&priv->defaults);
}

static void astal_wp_endpoint_finalize(GObject *object) {
    AstalWpEndpoint *self = ASTAL_WP_ENDPOINT(object);
    g_free(self->description);
}

static void astal_wp_endpoint_class_init(AstalWpEndpointClass *class) {
    GObjectClass *object_class = G_OBJECT_CLASS(class);
    object_class->dispose = astal_wp_endpoint_dispose;
    object_class->finalize = astal_wp_endpoint_finalize;
    object_class->get_property = astal_wp_endpoint_get_property;
    object_class->set_property = astal_wp_endpoint_set_property;

    astal_wp_endpoint_properties[ASTAL_WP_ENDPOINT_PROP_ID] =
        g_param_spec_uint("id", "id", "id", 0, UINT_MAX, 0, G_PARAM_READABLE);
    astal_wp_endpoint_properties[ASTAL_WP_ENDPOINT_PROP_VOLUME] =
        g_param_spec_double("volume", "volume", "volume", 0, 1, 0, G_PARAM_READWRITE);
    astal_wp_endpoint_properties[ASTAL_WP_ENDPOINT_PROP_MUTE] =
        g_param_spec_boolean("mute", "mute", "mute", TRUE, G_PARAM_READWRITE);
    astal_wp_endpoint_properties[ASTAL_WP_ENDPOINT_PROP_DESCRIPTION] =
        g_param_spec_string("description", "description", "description", NULL, G_PARAM_READABLE);
    /**
     * AstalWpEndpoint:media-class: (type AstalWpMediaClass)
     *
     * The media class of this endpoint
     */
    astal_wp_endpoint_properties[ASTAL_WP_ENDPOINT_PROP_MEDIA_CLASS] =
        g_param_spec_enum("media-class", "media-class", "media-class", ASTAL_WP_TYPE_MEDIA_CLASS, 1,
                          G_PARAM_READABLE);
    astal_wp_endpoint_properties[ASTAL_WP_ENDPOINT_PROP_DEFAULT] =
        g_param_spec_boolean("is_default", "is_default", "is_default", FALSE, G_PARAM_READWRITE);

    g_object_class_install_properties(object_class, ASTAL_WP_ENDPOINT_N_PROPERTIES,
                                      astal_wp_endpoint_properties);

    astal_wp_endpoint_signals[ASTAL_WP_ENDPOINT_SIGNAL_CHANGED] =
        g_signal_new("changed", G_TYPE_FROM_CLASS(class), G_SIGNAL_RUN_FIRST, 0, NULL, NULL, NULL,
                     G_TYPE_NONE, 0);
}
