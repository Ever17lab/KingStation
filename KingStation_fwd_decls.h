#ifndef _KingStation_FWD_DECLS_H
#define _KingStation_FWD_DECLS_H

#ifdef HAVE_DISCORD
#if defined(__cplusplus) && !defined(CXX_BUILD)
extern "C"
{
#endif
   void Discord_Register(const char *a, const char *b);
#if defined(__cplusplus) && !defined(CXX_BUILD)
}
#endif
#endif

static void KingStation_fail(int error_code, const char *error);
static void KingStation_core_options_intl_init(
      struct kingsn_state *p_kingsn,
      const struct
      ks_core_options_intl *core_options_intl);
static void ui_companion_driver_toggle(
      settings_t *settings,
      struct kingsn_state *p_kingsn,
      bool force);

#ifdef HAVE_ACCESSIBILITY
static void set_gamepad_input_override(struct kingsn_state *p_kingsn,
      unsigned i, bool val);
#endif

#ifdef HAVE_LIBNX
void libnx_apply_overclock(void);
#endif
#ifdef HAVE_ACCESSIBILITY
#ifdef HAVE_TRANSLATE
static bool is_narrator_running(struct kingsn_state *p_kingsn);
#endif
static bool accessibility_startup_message(struct kingsn_state *p_kingsn);
#endif

#ifdef HAVE_NETWORKING
static void deinit_netplay(struct kingsn_state *p_kingsn);
#endif

static void KingStation_deinit_drivers(struct kingsn_state *p_kingsn);

static bool midi_driver_read(uint8_t *byte);
static bool midi_driver_write(uint8_t byte, uint32_t delta_time);
static bool midi_driver_output_enabled(void);
static bool midi_driver_input_enabled(void);
static bool midi_driver_set_all_sounds_off(struct kingsn_state *p_kingsn);
static const void *midi_driver_find_handle(int index);
static bool midi_driver_flush(void);

static void KingStation_deinit_core_options(struct kingsn_state *p_kingsn);
static void KingStation_init_core_variables(
      struct kingsn_state *p_kingsn,
      const struct ks_variable *vars);
static void kingsn_init_core_options(
      struct kingsn_state *p_kingsn,
      const struct ks_core_option_definition *option_defs);
#ifdef HAVE_RUNAHEAD
#if defined(HAVE_DYNAMIC) || defined(HAVE_DYLIB)
static bool secondary_core_create(struct kingsn_state *p_kingsn);
#endif
static int16_t input_state_get_last(unsigned port,
      unsigned device, unsigned index, unsigned id);
#endif
static int16_t input_state(unsigned port, unsigned device,
      unsigned idx, unsigned id);
static void video_driver_frame(const void *data, unsigned width,
      unsigned height, size_t pitch);
static void ks_frame_null(const void *data, unsigned width,
      unsigned height, size_t pitch);
static void ks_run_null(void);
static void ks_input_poll_null(void);

static uint64_t input_driver_get_capabilities(void);

static void uninit_libks_symbols(
      struct kingsn_state *p_kingsn,
      struct ks_core_t *current_core);
static bool init_libks_symbols(
      struct kingsn_state *p_kingsn,
      enum kingsn_core_type type,
      struct ks_core_t *current_core);

static void ui_companion_driver_deinit(struct kingsn_state *p_kingsn);
static void ui_companion_driver_init_first(
      settings_t *settings,
      struct kingsn_state *p_kingsn);

static bool audio_driver_stop(struct kingsn_state *p_kingsn);
static bool audio_driver_start(struct kingsn_state *p_kingsn,
      bool is_shutdown);

static bool recording_init(settings_t *settings,
      struct kingsn_state *p_kingsn);
static bool recording_deinit(struct kingsn_state *p_kingsn);

#ifdef HAVE_OVERLAY
static void KingStation_overlay_init(struct kingsn_state *p_kingsn);
static void KingStation_overlay_deinit(struct kingsn_state *p_kingsn);
static void input_overlay_set_alpha_mod(struct kingsn_state *p_kingsn,
      input_overlay_t *ol, float mod);
static void input_overlay_set_scale_factor(struct kingsn_state *p_kingsn,
      input_overlay_t *ol, const overlay_layout_desc_t *layout_desc);
static void input_overlay_load_active(
      struct kingsn_state *p_kingsn,
      input_overlay_t *ol, float opacity);
static void input_overlay_auto_rotate_(struct kingsn_state *p_kingsn,
      input_overlay_t *ol);
#endif

#ifdef HAVE_AUDIOMIXER
static void audio_mixer_play_stop_sequential_cb(
      audio_mixer_sound_t *sound, unsigned reason);
static void audio_mixer_play_stop_cb(
      audio_mixer_sound_t *sound, unsigned reason);
static void audio_mixer_menu_stop_cb(
      audio_mixer_sound_t *sound, unsigned reason);
#endif

static void video_driver_gpu_record_deinit(struct kingsn_state *p_kingsn);
static ks_proc_address_t video_driver_get_proc_address(const char *sym);
static uintptr_t video_driver_get_current_framebuffer(void);
static bool video_driver_find_driver(struct kingsn_state *p_kingsn);

#ifdef HAVE_BSV_MOVIE
static void bsv_movie_deinit(struct kingsn_state *p_kingsn);
static bool bsv_movie_init(struct kingsn_state *p_kingsn);
static bool bsv_movie_check(struct kingsn_state *p_kingsn);
#endif

static void driver_uninit(struct kingsn_state *p_kingsn, int flags);
static void drivers_init(struct kingsn_state *p_kingsn,  int flags);

#if defined(HAVE_RUNAHEAD)
static void core_free_ks_game_info(struct ks_game_info *dest);
#endif
static bool core_load(struct kingsn_state *p_kingsn,
      unsigned poll_type_behavior);
static bool core_unload_game(struct kingsn_state *p_kingsn);

static bool kingsn_environment_cb(unsigned cmd, void *data);

static bool driver_location_get_position(double *lat, double *lon,
      double *horiz_accuracy, double *vert_accuracy);
static void driver_location_set_interval(unsigned interval_msecs,
      unsigned interval_distance);
static void driver_location_stop(void);
static bool driver_location_start(void);
static void driver_camera_stop(void);
static bool driver_camera_start(void);
static int16_t input_joypad_analog_button(
      struct kingsn_state *p_kingsn,
      settings_t *settings,
      const input_device_driver_t *drv,
      kingsn_joypad_info_t *joypad_info,
      unsigned port, unsigned idx, unsigned ident,
      const struct ks_keybind *binds);
static int16_t input_joypad_analog_axis(
      struct kingsn_state *p_kingsn,
      settings_t *settings,
      const input_device_driver_t *drv,
      kingsn_joypad_info_t *joypad_info,
      unsigned port, unsigned idx, unsigned ident,
      const struct ks_keybind *binds);

#ifdef HAVE_ACCESSIBILITY
static bool is_accessibility_enabled(struct kingsn_state *p_kingsn);
static bool accessibility_speak_priority(
      struct kingsn_state *p_kingsn,
      const char* speak_text, int priority);
#endif

#ifdef HAVE_MENU
static bool input_mouse_button_raw(
      struct kingsn_state *p_kingsn,
      unsigned port, unsigned id);
static void input_keyboard_line_append(
      struct kingsn_state *p_kingsn,
      const char *word);
static const char **input_keyboard_start_line(void *userdata,
      struct kingsn_state *p_kingsn,
      input_keyboard_line_complete_t cb);

static void menu_driver_list_free(
      struct kingsn_state *p_kingsn,
      menu_ctx_list_t *list);
static void menu_input_post_iterate(
      struct kingsn_state *p_kingsn,
      int *ret, unsigned action,
      ks_time_t current_time);
static void menu_input_reset(struct kingsn_state *p_kingsn);
#endif

static void video_driver_restore_cached(struct kingsn_state *p_kingsn,
      settings_t *settings);

static const void *find_driver_nonempty(
      const char *label, int i,
      char *s, size_t len);

#endif
