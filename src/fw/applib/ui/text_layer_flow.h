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

#pragma once

#include "text_layer.h"

//! @internal
//! Default value for paging if we cannot find a container layer (e.g. ScrollLayer).
//! The value is very large but small enough to prevent overflows in the math
//! in case the layer's origin is > 0
#define TEXT_LAYER_FLOW_DEFAULT_PAGING_HEIGHT (INT16_MAX / 2)

//! @internal
bool text_layer_calc_text_flow_paging_values(const TextLayer *text_layer,
                                             GPoint *content_origin_on_screen,
                                             GRect *page_rect_on_screen);
