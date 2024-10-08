#include "audio.h"

#include <wp/wp.h>

#include "device.h"
#include "endpoint.h"
#include "glib-object.h"
#include "wp.h"

struct _AstalWpAudio {
    GObject parent_instance;
};

typedef struct {
    AstalWpWp *wp;
} AstalWpAudioPrivate;

G_DEFINE_FINAL_TYPE_WITH_PRIVATE(AstalWpAudio, astal_wp_audio, G_TYPE_OBJECT);

typedef enum {
    ASTAL_WP_AUDIO_SIGNAL_MICROPHONE_ADDED,
    ASTAL_WP_AUDIO_SIGNAL_MICROPHONE_REMOVED,
    ASTAL_WP_AUDIO_SIGNAL_SPEAKER_ADDED,
    ASTAL_WP_AUDIO_SIGNAL_SPEAKER_REMOVED,
    ASTAL_WP_AUDIO_SIGNAL_STREAM_ADDED,
    ASTAL_WP_AUDIO_SIGNAL_STREAM_REMOVED,
    ASTAL_WP_AUDIO_SIGNAL_RECORDER_ADDED,
    ASTAL_WP_AUDIO_SIGNAL_RECORDER_REMOVED,
    ASTAL_WP_AUDIO_SIGNAL_DEVICE_ADDED,
    ASTAL_WP_AUDIO_SIGNAL_DEVICE_REMOVED,
    ASTAL_WP_AUDIO_N_SIGNALS
} AstalWpWpSignals;

static guint astal_wp_audio_signals[ASTAL_WP_AUDIO_N_SIGNALS] = {
    0,
};

typedef enum {
    ASTAL_WP_AUDIO_PROP_MICROPHONES = 1,
    ASTAL_WP_AUDIO_PROP_SPEAKERS,
    ASTAL_WP_AUDIO_PROP_STREAMS,
    ASTAL_WP_AUDIO_PROP_RECORDERS,
    ASTAL_WP_AUDIO_PROP_DEVICES,
    ASTAL_WP_AUDIO_PROP_DEFAULT_SPEAKER,
    ASTAL_WP_AUDIO_PROP_DEFAULT_MICROPHONE,
    ASTAL_WP_AUDIO_N_PROPERTIES,
} AstalWpAudioProperties;

static GParamSpec *astal_wp_audio_properties[ASTAL_WP_AUDIO_N_PROPERTIES] = {
    NULL,
};

/**
 * astal_wp_audio_get_speaker:
 * @self: the AstalWpAudio object
 * @id: the id of the endpoint
 *
 * gets the speaker with the given id
 *
 * Returns: (transfer none) (nullable)
 */
AstalWpEndpoint *astal_wp_audio_get_speaker(AstalWpAudio *self, guint id) {
    AstalWpAudioPrivate *priv = astal_wp_audio_get_instance_private(self);

    AstalWpEndpoint *endpoint = astal_wp_wp_get_endpoint(priv->wp, id);
    if (astal_wp_endpoint_get_media_class(endpoint) == ASTAL_WP_MEDIA_CLASS_AUDIO_SPEAKER)
        return endpoint;
    return NULL;
}

/**
 * astal_wp_audio_get_microphone:
 * @self: the AstalWpAudio object
 * @id: the id of the endpoint
 *
 * gets the microphone with the given id
 *
 * Returns: (transfer none) (nullable)
 */
AstalWpEndpoint *astal_wp_audio_get_microphone(AstalWpAudio *self, guint id) {
    AstalWpAudioPrivate *priv = astal_wp_audio_get_instance_private(self);

    AstalWpEndpoint *endpoint = astal_wp_wp_get_endpoint(priv->wp, id);
    if (astal_wp_endpoint_get_media_class(endpoint) == ASTAL_WP_MEDIA_CLASS_AUDIO_MICROPHONE)
        return endpoint;
    return NULL;
}

/**
 * astal_wp_audio_get_recorder:
 * @self: the AstalWpAudio object
 * @id: the id of the endpoint
 *
 * gets the recorder with the given id
 *
 * Returns: (transfer none) (nullable)
 */
AstalWpEndpoint *astal_wp_audio_get_recorder(AstalWpAudio *self, guint id) {
    AstalWpAudioPrivate *priv = astal_wp_audio_get_instance_private(self);

    AstalWpEndpoint *endpoint = astal_wp_wp_get_endpoint(priv->wp, id);
    if (astal_wp_endpoint_get_media_class(endpoint) == ASTAL_WP_MEDIA_CLASS_AUDIO_RECORDER)
        return endpoint;
    return NULL;
}

/**
 * astal_wp_audio_get_stream:
 * @self: the AstalWpAudio object
 * @id: the id of the endpoint
 *
 * gets the stream with the given id
 *
 * Returns: (transfer none) (nullable)
 */
AstalWpEndpoint *astal_wp_audio_get_stream(AstalWpAudio *self, guint id) {
    AstalWpAudioPrivate *priv = astal_wp_audio_get_instance_private(self);

    AstalWpEndpoint *endpoint = astal_wp_wp_get_endpoint(priv->wp, id);
    if (astal_wp_endpoint_get_media_class(endpoint) == ASTAL_WP_MEDIA_CLASS_AUDIO_STREAM)
        return endpoint;
    return NULL;
}

/**
 * astal_wp_audio_get_device:
 * @self: the AstalWpAudio object
 * @id: the id of the device
 *
 * gets the device with the given id
 *
 * Returns: (transfer none) (nullable)
 */
AstalWpDevice *astal_wp_audio_get_device(AstalWpAudio *self, guint id) {
    AstalWpAudioPrivate *priv = astal_wp_audio_get_instance_private(self);

    return astal_wp_wp_get_device(priv->wp, id);
}

/**
 * astal_wp_audio_get_microphones:
 * @self: the AstalWpAudio object
 *
 * a GList containing the microphones
 *
 * Returns: (transfer container) (nullable) (type GList(AstalWpEndpoint))
 */
GList *astal_wp_audio_get_microphones(AstalWpAudio *self) {
    AstalWpAudioPrivate *priv = astal_wp_audio_get_instance_private(self);
    GList *eps = astal_wp_wp_get_endpoints(priv->wp);
    GList *mics = NULL;

    for (GList *l = eps; l != NULL; l = l->next) {
        if (astal_wp_endpoint_get_media_class(l->data) == ASTAL_WP_MEDIA_CLASS_AUDIO_MICROPHONE) {
            mics = g_list_append(mics, l->data);
        }
    }
    g_list_free(eps);
    return mics;
}

/**
 * astal_wp_audio_get_speakers:
 * @self: the AstalWpAudio object
 *
 * a GList containing the speakers
 *
 * Returns: (transfer container) (nullable) (type GList(AstalWpEndpoint))
 */
GList *astal_wp_audio_get_speakers(AstalWpAudio *self) {
    AstalWpAudioPrivate *priv = astal_wp_audio_get_instance_private(self);
    GList *eps = astal_wp_wp_get_endpoints(priv->wp);
    GList *speakers = NULL;

    for (GList *l = eps; l != NULL; l = l->next) {
        if (astal_wp_endpoint_get_media_class(l->data) == ASTAL_WP_MEDIA_CLASS_AUDIO_SPEAKER) {
            speakers = g_list_append(speakers, l->data);
        }
    }
    g_list_free(eps);
    return speakers;
}

/**
 * astal_wp_audio_get_recorders:
 * @self: the AstalWpAudio object
 *
 * a GList containing the recorders
 *
 * Returns: (transfer container) (nullable) (type GList(AstalWpEndpoint))
 */
GList *astal_wp_audio_get_recorders(AstalWpAudio *self) {
    AstalWpAudioPrivate *priv = astal_wp_audio_get_instance_private(self);
    GList *eps = astal_wp_wp_get_endpoints(priv->wp);
    GList *recorders = NULL;

    for (GList *l = eps; l != NULL; l = l->next) {
        if (astal_wp_endpoint_get_media_class(l->data) == ASTAL_WP_MEDIA_CLASS_AUDIO_RECORDER) {
            recorders = g_list_append(recorders, l->data);
        }
    }
    g_list_free(eps);
    return recorders;
}

/**
 * astal_wp_audio_get_streams:
 * @self: the AstalWpAudio object
 *
 * a GList containing the streams
 *
 * Returns: (transfer container) (nullable) (type GList(AstalWpEndpoint))
 */
GList *astal_wp_audio_get_streams(AstalWpAudio *self) {
    AstalWpAudioPrivate *priv = astal_wp_audio_get_instance_private(self);
    GList *eps = astal_wp_wp_get_endpoints(priv->wp);
    GList *streams = NULL;

    for (GList *l = eps; l != NULL; l = l->next) {
        if (astal_wp_endpoint_get_media_class(l->data) == ASTAL_WP_MEDIA_CLASS_AUDIO_STREAM) {
            streams = g_list_append(streams, l->data);
        }
    }
    g_list_free(eps);
    return streams;
}

/**
 * astal_wp_audio_get_devices:
 * @self: the AstalWpAudio object
 *
 * a GList containing the devices
 *
 * Returns: (transfer container) (nullable) (type GList(AstalWpDevice))
 */
GList *astal_wp_audio_get_devices(AstalWpAudio *self) {
    AstalWpAudioPrivate *priv = astal_wp_audio_get_instance_private(self);
    GList *eps = astal_wp_wp_get_devices(priv->wp);
    GList *list = NULL;

    for (GList *l = eps; l != NULL; l = l->next) {
        if (astal_wp_device_get_device_type(l->data) == ASTAL_WP_DEVICE_TYPE_AUDIO) {
            list = g_list_append(list, l->data);
        }
    }
    g_list_free(eps);
    return list;
}

/**
 * astal_wp_audio_get_endpoint:
 * @self: the AstalWpAudio object
 * @id: the id of the endpoint
 *
 * the endpoint with the given id
 *
 * Returns: (transfer none) (nullable)
 */
AstalWpEndpoint *astal_wp_audio_get_endpoint(AstalWpAudio *self, guint id) {
    AstalWpAudioPrivate *priv = astal_wp_audio_get_instance_private(self);

    AstalWpEndpoint *endpoint = astal_wp_wp_get_endpoint(priv->wp, id);
    return endpoint;
}

/**
 * astal_wp_audio_get_default_speaker
 *
 * gets the default speaker object
 *
 * Returns: (nullable) (transfer none)
 */
AstalWpEndpoint *astal_wp_audio_get_default_speaker(AstalWpAudio *self) {
    AstalWpAudioPrivate *priv = astal_wp_audio_get_instance_private(self);
    return astal_wp_wp_get_default_speaker(priv->wp);
}

/**
 * astal_wp_audio_get_default_microphone
 *
 * gets the default microphone object
 *
 * Returns: (nullable) (transfer none)
 */
AstalWpEndpoint *astal_wp_audio_get_default_microphone(AstalWpAudio *self) {
    AstalWpAudioPrivate *priv = astal_wp_audio_get_instance_private(self);
    return astal_wp_wp_get_default_microphone(priv->wp);
}

static void astal_wp_audio_get_property(GObject *object, guint property_id, GValue *value,
                                        GParamSpec *pspec) {
    AstalWpAudio *self = ASTAL_WP_AUDIO(object);

    switch (property_id) {
        case ASTAL_WP_AUDIO_PROP_MICROPHONES:
            g_value_set_pointer(value, astal_wp_audio_get_microphones(self));
            break;
        case ASTAL_WP_AUDIO_PROP_SPEAKERS:
            g_value_set_pointer(value, astal_wp_audio_get_speakers(self));
            break;
        case ASTAL_WP_AUDIO_PROP_STREAMS:
            g_value_set_pointer(value, astal_wp_audio_get_streams(self));
            break;
        case ASTAL_WP_AUDIO_PROP_RECORDERS:
            g_value_set_pointer(value, astal_wp_audio_get_recorders(self));
            break;
        case ASTAL_WP_AUDIO_PROP_DEFAULT_SPEAKER:
            g_value_set_object(value, astal_wp_audio_get_default_speaker(self));
            break;
        case ASTAL_WP_AUDIO_PROP_DEVICES:
            g_value_set_pointer(value, astal_wp_audio_get_devices(self));
            break;
        case ASTAL_WP_AUDIO_PROP_DEFAULT_MICROPHONE:
            g_value_set_object(value, astal_wp_audio_get_default_microphone(self));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void astal_wp_audio_device_added(AstalWpAudio *self, gpointer object) {
    AstalWpDevice *device = ASTAL_WP_DEVICE(object);
    if (astal_wp_device_get_device_type(device) == ASTAL_WP_DEVICE_TYPE_AUDIO) {
        g_signal_emit_by_name(self, "device-added", device);
        g_object_notify(G_OBJECT(self), "devices");
    }
}

static void astal_wp_audio_device_removed(AstalWpAudio *self, gpointer object) {
    AstalWpDevice *device = ASTAL_WP_DEVICE(object);
    if (astal_wp_device_get_device_type(device) == ASTAL_WP_DEVICE_TYPE_AUDIO) {
        g_signal_emit_by_name(self, "device-removed", device);
        g_object_notify(G_OBJECT(self), "devices");
    }
}

static void astal_wp_audio_object_added(AstalWpAudio *self, gpointer object) {
    AstalWpEndpoint *endpoint = ASTAL_WP_ENDPOINT(object);
    switch (astal_wp_endpoint_get_media_class(endpoint)) {
        case ASTAL_WP_MEDIA_CLASS_AUDIO_MICROPHONE:
            g_signal_emit_by_name(self, "microphone-added", endpoint);
            g_object_notify(G_OBJECT(self), "microphones");
            break;
        case ASTAL_WP_MEDIA_CLASS_AUDIO_SPEAKER:
            g_signal_emit_by_name(self, "speaker-added", endpoint);
            g_object_notify(G_OBJECT(self), "speakers");
            break;
        case ASTAL_WP_MEDIA_CLASS_AUDIO_STREAM:
            g_signal_emit_by_name(self, "stream-added", endpoint);
            g_object_notify(G_OBJECT(self), "streams");
            break;
        case ASTAL_WP_MEDIA_CLASS_AUDIO_RECORDER:
            g_signal_emit_by_name(self, "recorder-added", endpoint);
            g_object_notify(G_OBJECT(self), "recorders");
            break;
        default:
            break;
    }
}

static void astal_wp_audio_object_removed(AstalWpAudio *self, gpointer object) {
    AstalWpEndpoint *endpoint = ASTAL_WP_ENDPOINT(object);
    switch (astal_wp_endpoint_get_media_class(endpoint)) {
        case ASTAL_WP_MEDIA_CLASS_AUDIO_MICROPHONE:
            g_signal_emit_by_name(self, "microphone-removed", endpoint);
            g_object_notify(G_OBJECT(self), "microphones");
            break;
        case ASTAL_WP_MEDIA_CLASS_AUDIO_SPEAKER:
            g_signal_emit_by_name(self, "speaker-removed", endpoint);
            g_object_notify(G_OBJECT(self), "speakers");
            break;
        case ASTAL_WP_MEDIA_CLASS_AUDIO_STREAM:
            g_signal_emit_by_name(self, "stream-removed", endpoint);
            g_object_notify(G_OBJECT(self), "streams");
            break;
        case ASTAL_WP_MEDIA_CLASS_AUDIO_RECORDER:
            g_signal_emit_by_name(self, "recorder-removed", endpoint);
            g_object_notify(G_OBJECT(self), "recorders");
            break;
        default:
            break;
    }
}

AstalWpAudio *astal_wp_audio_new(AstalWpWp *wp) {
    AstalWpAudio *self = g_object_new(ASTAL_WP_TYPE_AUDIO, NULL);
    AstalWpAudioPrivate *priv = astal_wp_audio_get_instance_private(self);
    priv->wp = g_object_ref(wp);

    g_signal_connect_swapped(priv->wp, "endpoint-added", G_CALLBACK(astal_wp_audio_object_added),
                             self);
    g_signal_connect_swapped(priv->wp, "endpoint-removed",
                             G_CALLBACK(astal_wp_audio_object_removed), self);
    g_signal_connect_swapped(priv->wp, "device-added", G_CALLBACK(astal_wp_audio_device_added),
                             self);
    g_signal_connect_swapped(priv->wp, "device-removed", G_CALLBACK(astal_wp_audio_device_removed),
                             self);

    return self;
}

static void astal_wp_audio_dispose(GObject *object) {
    AstalWpAudio *self = ASTAL_WP_AUDIO(object);
    AstalWpAudioPrivate *priv = astal_wp_audio_get_instance_private(self);
    g_clear_object(&priv->wp);
}

static void astal_wp_audio_init(AstalWpAudio *self) {
    AstalWpAudioPrivate *priv = astal_wp_audio_get_instance_private(self);
}

static void astal_wp_audio_class_init(AstalWpAudioClass *class) {
    GObjectClass *object_class = G_OBJECT_CLASS(class);
    object_class->get_property = astal_wp_audio_get_property;
    object_class->dispose = astal_wp_audio_dispose;

    /**
     * AstalWpAudio:microphones: (type GList(AstalWpEndpoint)) (transfer container)
     *
     * A list of AstalWpEndpoint objects
     */
    astal_wp_audio_properties[ASTAL_WP_AUDIO_PROP_MICROPHONES] =
        g_param_spec_pointer("microphones", "microphones", "microphones", G_PARAM_READABLE);
    /**
     * AstalWpAudio:speakers: (type GList(AstalWpEndpoint)) (transfer container)
     *
     * A list of AstalWpEndpoint objects
     */
    astal_wp_audio_properties[ASTAL_WP_AUDIO_PROP_SPEAKERS] =
        g_param_spec_pointer("speakers", "speakers", "speakers", G_PARAM_READABLE);
    /**
     * AstalWpAudio:recorders: (type GList(AstalWpEndpoint)) (transfer container)
     *
     * A list of AstalWpEndpoint objects
     */
    astal_wp_audio_properties[ASTAL_WP_AUDIO_PROP_RECORDERS] =
        g_param_spec_pointer("recorders", "recorders", "recorders", G_PARAM_READABLE);
    /**
     * AstalWpAudio:streams: (type GList(AstalWpEndpoint)) (transfer container)
     *
     * A list of AstalWpEndpoint objects
     */
    astal_wp_audio_properties[ASTAL_WP_AUDIO_PROP_STREAMS] =
        g_param_spec_pointer("streams", "streams", "streams", G_PARAM_READABLE);
    /**
     * AstalWpAudio:devices: (type GList(AstalWpDevice)) (transfer container)
     *
     * A list of AstalWpEndpoint objects
     */
    astal_wp_audio_properties[ASTAL_WP_AUDIO_PROP_DEVICES] =
        g_param_spec_pointer("devices", "devices", "devices", G_PARAM_READABLE);
    /**
     * AstalWpAudio:default-speaker:
     *
     * The AstalWndpoint object representing the default speaker
     */
    astal_wp_audio_properties[ASTAL_WP_AUDIO_PROP_DEFAULT_SPEAKER] =
        g_param_spec_object("default-speaker", "default-speaker", "default-speaker",
                            ASTAL_WP_TYPE_ENDPOINT, G_PARAM_READABLE);
    /**
     * AstalWpAudio:default-microphone:
     *
     * The AstalWndpoint object representing the default speaker
     */
    astal_wp_audio_properties[ASTAL_WP_AUDIO_PROP_DEFAULT_MICROPHONE] =
        g_param_spec_object("default-microphone", "default-microphone", "default-microphone",
                            ASTAL_WP_TYPE_ENDPOINT, G_PARAM_READABLE);

    g_object_class_install_properties(object_class, ASTAL_WP_AUDIO_N_PROPERTIES,
                                      astal_wp_audio_properties);

    astal_wp_audio_signals[ASTAL_WP_AUDIO_SIGNAL_MICROPHONE_ADDED] =
        g_signal_new("microphone-added", G_TYPE_FROM_CLASS(class), G_SIGNAL_RUN_FIRST, 0, NULL,
                     NULL, NULL, G_TYPE_NONE, 1, ASTAL_WP_TYPE_ENDPOINT);
    astal_wp_audio_signals[ASTAL_WP_AUDIO_SIGNAL_MICROPHONE_REMOVED] =
        g_signal_new("microphone-removed", G_TYPE_FROM_CLASS(class), G_SIGNAL_RUN_FIRST, 0, NULL,
                     NULL, NULL, G_TYPE_NONE, 1, ASTAL_WP_TYPE_ENDPOINT);
    astal_wp_audio_signals[ASTAL_WP_AUDIO_SIGNAL_SPEAKER_ADDED] =
        g_signal_new("speaker-added", G_TYPE_FROM_CLASS(class), G_SIGNAL_RUN_FIRST, 0, NULL, NULL,
                     NULL, G_TYPE_NONE, 1, ASTAL_WP_TYPE_ENDPOINT);
    astal_wp_audio_signals[ASTAL_WP_AUDIO_SIGNAL_SPEAKER_REMOVED] =
        g_signal_new("speaker-removed", G_TYPE_FROM_CLASS(class), G_SIGNAL_RUN_FIRST, 0, NULL, NULL,
                     NULL, G_TYPE_NONE, 1, ASTAL_WP_TYPE_ENDPOINT);
    astal_wp_audio_signals[ASTAL_WP_AUDIO_SIGNAL_STREAM_ADDED] =
        g_signal_new("stream-added", G_TYPE_FROM_CLASS(class), G_SIGNAL_RUN_FIRST, 0, NULL, NULL,
                     NULL, G_TYPE_NONE, 1, ASTAL_WP_TYPE_ENDPOINT);
    astal_wp_audio_signals[ASTAL_WP_AUDIO_SIGNAL_STREAM_REMOVED] =
        g_signal_new("stream-removed", G_TYPE_FROM_CLASS(class), G_SIGNAL_RUN_FIRST, 0, NULL, NULL,
                     NULL, G_TYPE_NONE, 1, ASTAL_WP_TYPE_ENDPOINT);
    astal_wp_audio_signals[ASTAL_WP_AUDIO_SIGNAL_RECORDER_ADDED] =
        g_signal_new("recorder-added", G_TYPE_FROM_CLASS(class), G_SIGNAL_RUN_FIRST, 0, NULL, NULL,
                     NULL, G_TYPE_NONE, 1, ASTAL_WP_TYPE_ENDPOINT);
    astal_wp_audio_signals[ASTAL_WP_AUDIO_SIGNAL_RECORDER_REMOVED] =
        g_signal_new("recorder-removed", G_TYPE_FROM_CLASS(class), G_SIGNAL_RUN_FIRST, 0, NULL,
                     NULL, NULL, G_TYPE_NONE, 1, ASTAL_WP_TYPE_ENDPOINT);
    astal_wp_audio_signals[ASTAL_WP_AUDIO_SIGNAL_DEVICE_ADDED] =
        g_signal_new("device-added", G_TYPE_FROM_CLASS(class), G_SIGNAL_RUN_FIRST, 0, NULL, NULL,
                     NULL, G_TYPE_NONE, 1, ASTAL_WP_TYPE_DEVICE);
    astal_wp_audio_signals[ASTAL_WP_AUDIO_SIGNAL_MICROPHONE_REMOVED] =
        g_signal_new("device-removed", G_TYPE_FROM_CLASS(class), G_SIGNAL_RUN_FIRST, 0, NULL, NULL,
                     NULL, G_TYPE_NONE, 1, ASTAL_WP_TYPE_DEVICE);
}
