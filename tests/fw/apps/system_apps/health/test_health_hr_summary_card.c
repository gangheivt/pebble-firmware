/*
 * Copyright 2024 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "apps/system_apps/health/health_data.h"
#include "apps/system_apps/health/health_data_private.h"
#include "apps/system_apps/health/health_hr_summary_card.h"
#include "apps/system_apps/health/health_hr_detail_card.h"
#include "apps/system_apps/health/health_detail_card.h"

#include "test_health_app_includes.h"

// Fakes
////////////////////////////////////

void clock_get_until_time_without_fulltime(char *buffer, int buf_size, time_t timestamp,
                                           int max_relative_hrs) {
  snprintf(buffer, buf_size, "5 MIN AGO");
}

// Setup and Teardown
////////////////////////////////////

static GContext s_ctx;
static FrameBuffer s_fb;

GContext *graphics_context_get_current_context(void) {
  return &s_ctx;
}

void test_health_hr_summary_card__initialize(void) {
  // Setup graphics context
  framebuffer_init(&s_fb, &(GSize) {DISP_COLS, DISP_ROWS});
  framebuffer_clear(&s_fb);
  graphics_context_init(&s_ctx, &s_fb, GContextInitializationMode_App);
  s_app_state_get_graphics_context = &s_ctx;

  // Setup resources
  fake_spi_flash_init(0 /* offset */, 0x1000000 /* length */);
  pfs_init(false /* run filesystem check */);
  pfs_format(true /* write erase headers */);
  load_resource_fixture_in_flash(RESOURCES_FIXTURE_PATH, SYSTEM_RESOURCES_FIXTURE_NAME,
                                 false /* is_next */);
  resource_init();

  // Setup content indicator
  ContentIndicatorsBuffer *buffer = content_indicator_get_current_buffer();
  content_indicator_init_buffer(buffer);
}

void test_health_hr_summary_card__cleanup(void) {
}

// Helpers
//////////////////////

static void prv_create_card_and_render(HealthData *health_data) {
  Window window;
  window_init(&window, WINDOW_NAME("Health"));
  Layer *window_layer = window_get_root_layer(&window);
  Layer *card_layer = health_hr_summary_card_create(health_data);
  layer_set_frame(card_layer, &window_layer->bounds);
  layer_add_child(window_layer, card_layer);
  window_set_background_color(&window, health_hr_summary_card_get_bg_color(card_layer));
  window_set_on_screen(&window, true, true);
  window_render(&window, &s_ctx);
}

// Tests
//////////////////////

void test_health_hr_summary_card__render_no_data(void) {
  prv_create_card_and_render(&(HealthData) {});
  cl_check(gbitmap_pbi_eq(&s_ctx.dest_bitmap, TEST_PBI_FILE));
}

void test_health_hr_summary_card__render_current_bpm(void) {
  HealthData health_data = {
    .current_hr_bpm = 110,
  };

  prv_create_card_and_render(&health_data);
  cl_check(gbitmap_pbi_eq(&s_ctx.dest_bitmap, TEST_PBI_FILE));
}

void test_health_hr_summary_card__render_timestamp(void) {
  rtc_set_time(SECONDS_PER_DAY + (SECONDS_PER_HOUR * 12));

  HealthData health_data = {
    .current_hr_bpm = 110,
    .hr_last_updated = rtc_get_time() - (SECONDS_PER_MINUTE * 5),
  };

  prv_create_card_and_render(&health_data);
  cl_check(gbitmap_pbi_eq(&s_ctx.dest_bitmap, TEST_PBI_FILE));
}
