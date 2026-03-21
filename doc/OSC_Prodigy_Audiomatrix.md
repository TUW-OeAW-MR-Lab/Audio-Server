# Prodigy.MP audio matrix OSC control interface

This file describes the OSC interface for controlling the Prodigy audio
matrix via the audio server, along with a list of supported OSC messages.

The conventions of OSC audio server control that also apply here are
to be found in the [OSC_Audioserver](OSC_Audioserver.md) document. All
OSC messages described here are of the `STATE` or `STATE_RW` kinds.

## General mechanism of Prodigy audio matrix OSC control

The Prodigy audio matrix exposes its internal state for networked
control through a JSON interface as documented in *DirectOut GmbH JSON
Protocol Specifications 1.0.* The audio server implements this
protocol and provides a one-to-one mapping of the Prodigy matrix JSON
state tree to an OSC address space. The actual state values, i.e.,
primitive JSON values at the leaves of the state tree, are translated
to OSC messages with an OSC address reflecting the place of the value
in the state hierarchy and an argument corresponding to the type of
the state value.

The OSC interface to the Prodigy state is "mounted" into the audio
server's OSC address space below the `/matrix` top-level OSC
address. Hence, all OSC messages whose address begins with `/matrix`
directly refer to the Prodigy audio matrix state. For the same reason,
OSC addresses that maintain the audio server's network connection
state to the matrix or apply scene changes that indirectly affect the
matrix state are *not* part of the `/matrix` namespace (refer to the
[OSC_Audioserver documentation](OSC_Audioserver.md) for details).

## Controlling the Prodigy audio matrix by OSC

The current value of a state node is queried by sending an OSC message
without arguments (`STATE`):

```
/matrix/settings/flex_channel/0/gain
```

The audio server will respond with a message with the same OSC address
and the current state as the (single) argument, whose type is
indicated in the list below.

Nodes in the `settings` subtree of the Prodigy state have read/write
access (`STATE_RW`). A state value can be set by sending an OSC
message with an argument of the respective type:

```
/matrix/settings/flex_channel/0/gain -12.0
```

No response will be sent by the audio server but the state change will
be propagated to all other clients currently connected.

If the state changes on the audio matrix, e.g., by loading a snapshot
or via the *globcon* control software, all audio server clients will
receive state update OSC messages.

### OSC wildcard matching

Wildcard matching as described in the
[OSC specification](https://opensoundcontrol.stanford.edu/spec-1_0.html#osc-message-dispatching-and-pattern-matching)
is supported for controlling the Prodigy audio matrix.

This will set the gain of all flex channels:

```
/matrix/settings/flex_channel/*/gain -12.0
```

This will query the current gain of all flex channels:

```
/matrix/settings/flex_channel/*/gain
```

The state query will trigger a response of 32 separate state update
messages, one for each flex channel.

## Prodigy audio matrix OSC address space reference

As described above, all audio matrix OSC control messages are
organised under the `/matrix` prefix. The address list directly
corresponds to the hierarchy of the Prodigy JSON state tree.

Numerical OSC address segments that refer to an instance of multiple
control entities (e.g., flex channels) are indicated by an inclusive
range, i.e., `[0-31]` means that `0` is the lowest and `31` is the
highest supported instance number.

### Read/write OSC addresses (`STATE_RW`, `settings` subtree)

```
/matrix/settings/automix/[0-15]/bias <float>
/matrix/settings/automix/[0-15]/label <string>
/matrix/settings/automix/[0-15]/speed <int>

/matrix/settings/clock_source/auto_clock/[0-17]/auto_resume <bool>
/matrix/settings/clock_source/auto_clock/[0-17]/penalty <bool>
/matrix/settings/clock_source/auto_clock/[0-17]/priority <int>
/matrix/settings/clock_source/display_stable_hold <bool>
/matrix/settings/clock_source/follow_range <bool>
/matrix/settings/clock_source/manual <int>
/matrix/settings/clock_source/multiplier <int>

/matrix/settings/color <string>

/matrix/settings/compressor/[0-31]/attack <float>
/matrix/settings/compressor/[0-31]/comp_out <float>
/matrix/settings/compressor/[0-31]/dry_out <float>
/matrix/settings/compressor/[0-31]/enabled <bool>
/matrix/settings/compressor/[0-31]/exp_attack <float>
/matrix/settings/compressor/[0-31]/exp_enabled <bool>
/matrix/settings/compressor/[0-31]/exp_hold <float>
/matrix/settings/compressor/[0-31]/exp_max_gain <int>
/matrix/settings/compressor/[0-31]/exp_ratio <float>
/matrix/settings/compressor/[0-31]/exp_release <float>
/matrix/settings/compressor/[0-31]/exp_threshold <float>
/matrix/settings/compressor/[0-31]/gate_out <float>
/matrix/settings/compressor/[0-31]/hold <float>
/matrix/settings/compressor/[0-31]/mode <int>
/matrix/settings/compressor/[0-31]/ratio <float>
/matrix/settings/compressor/[0-31]/release <float>
/matrix/settings/compressor/[0-31]/side_chain_enabled <bool>
/matrix/settings/compressor/[0-31]/side_chain_key <int>
/matrix/settings/compressor/[0-31]/softness <int>
/matrix/settings/compressor/[0-31]/stereo <bool>
/matrix/settings/compressor/[0-31]/threshold <float>

/matrix/settings/delay/[0-31]/enabled <bool>
/matrix/settings/delay/[0-31]/samples <int>
/matrix/settings/delay/[0-31]/unit <int>

/matrix/settings/dsp_mute_group/0 <int>

/matrix/settings/ears/[0-5]/dr <int>
/matrix/settings/ears/[0-5]/force <int>
/matrix/settings/ears/[0-5]/port <int>
/matrix/settings/ears/[0-5]/priority <int>

/matrix/settings/easy_routing/[0-451] <int>

/matrix/settings/eq/[0-31]/custom_filter/bypass_high_limit <bool>
/matrix/settings/eq/[0-31]/custom_filter/bypass_low_limit <bool>
/matrix/settings/eq/[0-31]/custom_filter/contribution <float>
/matrix/settings/eq/[0-31]/custom_filter/gain <float>
/matrix/settings/eq/[0-31]/custom_filter/high_ecf <float>
/matrix/settings/eq/[0-31]/custom_filter/high_frequency <float>
/matrix/settings/eq/[0-31]/custom_filter/inverted <bool>
/matrix/settings/eq/[0-31]/custom_filter/low_ecf <float>
/matrix/settings/eq/[0-31]/custom_filter/low_frequency <float>
/matrix/settings/eq/[0-31]/custom_filter/max_boost <float>
/matrix/settings/eq/[0-31]/custom_filter/plot <bool>
/matrix/settings/eq/[0-31]/custom_filter/points <string>
/matrix/settings/eq/[0-31]/custom_filter/smoothing <int>
/matrix/settings/eq/[0-31]/custom_layer <bool>
/matrix/settings/eq/[0-31]/enabled <bool>
/matrix/settings/eq/[0-31]/enabled_layer/[0-2] <bool>
/matrix/settings/eq/[0-31]/filter/[0-31]/enabled <bool>
/matrix/settings/eq/[0-31]/filter/[0-31]/f1 <float>
/matrix/settings/eq/[0-31]/filter/[0-31]/f2 <float>
/matrix/settings/eq/[0-31]/filter/[0-31]/gain <float>
/matrix/settings/eq/[0-31]/filter/[0-31]/layer <int>
/matrix/settings/eq/[0-31]/filter/[0-31]/q1 <float>
/matrix/settings/eq/[0-31]/filter/[0-31]/q2 <float>
/matrix/settings/eq/[0-31]/filter/[0-31]/type <int>
/matrix/settings/eq/[0-31]/gain <float>
/matrix/settings/eq/[0-31]/group_layer <bool>

/matrix/settings/flex_channel/[0-31]/automix_contrib <int>
/matrix/settings/flex_channel/[0-31]/automix_group <int>
/matrix/settings/flex_channel/[0-31]/automix_priority <float>
/matrix/settings/flex_channel/[0-31]/gain <float>
/matrix/settings/flex_channel/[0-31]/label <string>
/matrix/settings/flex_channel/[0-31]/meter <int>
/matrix/settings/flex_channel/[0-31]/mode <int>
/matrix/settings/flex_channel/[0-31]/mute <bool>
/matrix/settings/flex_channel/[0-31]/plugins/[0-3]/bypass <bool>
/matrix/settings/flex_channel/[0-31]/plugins/[0-3]/index <int>
/matrix/settings/flex_channel/[0-31]/plugins/[0-3]/type <int>
/matrix/settings/flex_channel/[0-31]/polarity <int>
/matrix/settings/flex_channel/[0-31]/source_routing <int>
/matrix/settings/flex_channel/[0-31]/time_adj <int>

/matrix/settings/gpio_polarity/[0-3] <bool>

/matrix/settings/gpo/[0-1] <bool>

/matrix/settings/group/channel/[0-15]/fade_in_time <int>
/matrix/settings/group/channel/[0-15]/fade_out <bool>
/matrix/settings/group/channel/[0-15]/fade_out_time <int>
/matrix/settings/group/channel/[0-15]/fading_state <int>
/matrix/settings/group/channel/[0-15]/gain <float>
/matrix/settings/group/channel/[0-15]/label <string>
/matrix/settings/group/channel/[0-15]/mute <bool>
/matrix/settings/group/channel/[0-15]/target_level <float>

/matrix/settings/group/delay/[0-15]/enabled <bool>
/matrix/settings/group/delay/[0-15]/samples <int>
/matrix/settings/group/delay/[0-15]/unit <int>
/matrix/settings/group/delay_assignment/[0-31] <int>

/matrix/settings/group/dsp_assignment/[0-319] <int>

/matrix/settings/group/eq_fir/[0-15]/enabled <bool>
/matrix/settings/group/eq_fir/[0-15]/enabled_layer/[0-2] <bool>
/matrix/settings/group/eq_fir/[0-15]/filter/[0-31]/enabled <bool>
/matrix/settings/group/eq_fir/[0-15]/filter/[0-31]/f1 <float>
/matrix/settings/group/eq_fir/[0-15]/filter/[0-31]/f2 <float>
/matrix/settings/group/eq_fir/[0-15]/filter/[0-31]/gain <float>
/matrix/settings/group/eq_fir/[0-15]/filter/[0-31]/layer <int>
/matrix/settings/group/eq_fir/[0-15]/filter/[0-31]/q1 <float>
/matrix/settings/group/eq_fir/[0-15]/filter/[0-31]/q2 <float>
/matrix/settings/group/eq_fir/[0-15]/filter/[0-31]/type <int>
/matrix/settings/group/eq_fir/[0-15]/gain <float>
/matrix/settings/group/eq_fir_assignment/[0-31] <int>

/matrix/settings/group/fading_coefficient <float>
/matrix/settings/group/output_assignment/[0-451] <int>

/matrix/settings/iir_eq/[0-31]/enabled <bool>
/matrix/settings/iir_eq/[0-31]/filter/[0-5]/enabled <bool>
/matrix/settings/iir_eq/[0-31]/filter/[0-5]/f <float>
/matrix/settings/iir_eq/[0-31]/filter/[0-5]/gain <float>
/matrix/settings/iir_eq/[0-31]/filter/[0-5]/q <float>
/matrix/settings/iir_eq/[0-31]/filter/[0-5]/type <int>

/matrix/settings/input_labels/[0-448] <string>
/matrix/settings/input_manager/[0-31]/coherency_detection <bool>
/matrix/settings/input_manager/[0-31]/coherency_th <int>
/matrix/settings/input_manager/[0-31]/enable_time <int>
/matrix/settings/input_manager/[0-31]/entries/[0-5]/auto_resume <bool>
/matrix/settings/input_manager/[0-31]/entries/[0-5]/penalty <bool>
/matrix/settings/input_manager/[0-31]/entries/[0-5]/priority <int>
/matrix/settings/input_manager/[0-31]/entries/[0-5]/source <int>
/matrix/settings/input_manager/[0-31]/failure_time <int>
/matrix/settings/input_manager/[0-31]/label <string>
/matrix/settings/input_manager/[0-31]/manual <int>
/matrix/settings/input_manager/[0-31]/mode <int>
/matrix/settings/input_manager/[0-31]/stereo <bool>
/matrix/settings/input_manager/[0-31]/threshold <float>
/matrix/settings/input_mute/[0-448] <int>
/matrix/settings/input_polarity/[0-448] <int>
/matrix/settings/input_slot_name/[0-11] <string>
/matrix/settings/input_trim/[0-448] <float>

/matrix/settings/insert/[0-127]/name <string>
/matrix/settings/insert/[0-127]/return <int>
/matrix/settings/insert/[0-127]/send <int>

/matrix/settings/link/enable/[0-9] <int>
/matrix/settings/link/level/[0-9] <int>
/matrix/settings/link/mute/[0-9] <int>
/matrix/settings/link/plugin_1/[0-9] <int>
/matrix/settings/link/plugin_2/[0-9] <int>
/matrix/settings/link/plugin_3/[0-9] <int>
/matrix/settings/link/plugin_4/[0-9] <int>

/matrix/settings/loudness/channels/[0-7]/active <bool>
/matrix/settings/loudness/channels/[0-7]/label <string>
/matrix/settings/loudness/channels/[0-7]/scale <int>
/matrix/settings/loudness/channels/[0-7]/source <int>
/matrix/settings/loudness/channels/[0-7]/unit <int>
/matrix/settings/loudness/mode <int>

/matrix/settings/ltc_source <int>

/matrix/settings/madi_netbase/[0-1]/redundancy/force_source <int>
/matrix/settings/madi_netbase/[0-1]/redundancy/mode <int>
/matrix/settings/madi_netbase/[0-1]/tx/[0-1]/bit_transparent <bool>
/matrix/settings/madi_netbase/[0-1]/tx/[0-1]/ch_mode <int>
/matrix/settings/madi_netbase/[0-1]/tx/[0-1]/frame_96k <bool>
/matrix/settings/madi_netbase/[0-1]/tx/[0-1]/legacy <bool>
/matrix/settings/madi_netbase/[0-1]/tx/[0-1]/out_dark <bool>
/matrix/settings/madi_netbase/[0-1]/tx/[0-1]/src_fs <int>
/matrix/settings/madi_netbase/[0-1]/tx/[0-1]/src_in <bool>
/matrix/settings/madi_netbase/[0-1]/tx/[0-1]/src_out <bool>

/matrix/settings/madi_redundancy/force_source <int>
/matrix/settings/madi_redundancy/mode <int>
/matrix/settings/madi_redundancy/to_gpo <bool>

/matrix/settings/madi_status_tx/[0-1]/auto_src <bool>
/matrix/settings/madi_status_tx/[0-1]/bit_transparent <bool>
/matrix/settings/madi_status_tx/[0-1]/ch_mode <int>
/matrix/settings/madi_status_tx/[0-1]/frame_96k <bool>
/matrix/settings/madi_status_tx/[0-1]/legacy <bool>
/matrix/settings/madi_status_tx/[0-1]/out_dark <bool>
/matrix/settings/madi_status_tx/[0-1]/src_bypass <bool>
/matrix/settings/madi_status_tx/[0-1]/src_fs <int>
/matrix/settings/madi_status_tx/[0-1]/src_in <bool>
/matrix/settings/madi_status_tx/[0-1]/src_out <bool>

/matrix/settings/midi_matrix/[0-3] <int>
/matrix/settings/midi_protocol <int>
/matrix/settings/midi_pt_offset <bool>

/matrix/settings/mixer/[0-31]/bussing <int>
/matrix/settings/mixer/[0-31]/label <string>
/matrix/settings/mixer/[0-31]/master/[0-7]/gain <float>
/matrix/settings/mixer/[0-31]/master/[0-7]/label <string>
/matrix/settings/mixer/[0-31]/master/[0-7]/meter <int>
/matrix/settings/mixer/[0-31]/master/[0-7]/mode <int>
/matrix/settings/mixer/[0-31]/master/[0-7]/mute <bool>
/matrix/settings/mixer/[0-31]/master/[0-7]/plugins/[0-3]/bypass <bool>
/matrix/settings/mixer/[0-31]/master/[0-7]/plugins/[0-3]/index <int>
/matrix/settings/mixer/[0-31]/master/[0-7]/plugins/[0-3]/type <int>
/matrix/settings/mixer/[0-31]/master/[0-7]/polarity <int>
/matrix/settings/mixer/[0-31]/mode <int>
/matrix/settings/mixer/[0-31]/mute/[0-63] <int>
/matrix/settings/mixer/[0-31]/pan_law <int>
/matrix/settings/mixer/[0-31]/param/[0-63] <float>
/matrix/settings/mixer/[0-31]/source_label/[0-7] <string>
/matrix/settings/mixer/[0-31]/source_routing/[0-7] <int>

/matrix/settings/mode <int>

/matrix/settings/monitoring/label/[0-3] <string>
/matrix/settings/monitoring/monitor_1/[0-1] <int>
/matrix/settings/monitoring/monitor_2/[0-1] <int>
/matrix/settings/monitoring/monitoring_mode/[0-3] <int>
/matrix/settings/monitoring/routing/[0-3] <int>
/matrix/settings/monitoring/solo_bus_enabled <bool>

/matrix/settings/mute/[0-451] <int>
/matrix/settings/mute_group/[0-4] <int>

/matrix/settings/net_status_tx_t/[0-1]/auto_src <bool>
/matrix/settings/net_status_tx_t/[0-1]/bit_transparent <bool>
/matrix/settings/net_status_tx_t/[0-1]/netbase_src <bool>
/matrix/settings/net_status_tx_t/[0-1]/src <bool>
/matrix/settings/net_status_tx_t/[0-1]/src_bypass <bool>

/matrix/settings/output_gain/[0-451] <float>
/matrix/settings/output_labels/[0-451] <string>
/matrix/settings/output_polarity/[0-451] <int>
/matrix/settings/output_trim/[0-451] <float>

/matrix/settings/pink_noise_generator/[0-1]/enabled <bool>
/matrix/settings/pink_noise_generator/[0-1]/gain <float>
/matrix/settings/sine_generator/[0-1]/enabled <bool>
/matrix/settings/sine_generator/[0-1]/f <float>
/matrix/settings/sine_generator/[0-1]/gain <float>

/matrix/settings/slot_bit_transparent <bool>
/matrix/settings/slot_name/[0-11] <string>
/matrix/settings/slot_settings/[0-7]/channels/[0-7]/analog_gain <int>
/matrix/settings/slot_settings/[0-7]/channels/[0-7]/p48 <bool>
/matrix/settings/slot_settings/[0-7]/channels/[0-7]/pad <bool>
/matrix/settings/slot_settings/[0-7]/src/[0-3] <bool>

/matrix/settings/sum_bus_assign_dsp/[0-31]/segment/[0-55] <int>
/matrix/settings/sum_bus_assign_io/[0-31]/segment/[0-15] <int>
/matrix/settings/sum_bus_master/[0-31]/gain <float>
/matrix/settings/sum_bus_master/[0-31]/label <string>
/matrix/settings/sum_bus_master/[0-31]/meter <int>
/matrix/settings/sum_bus_master/[0-31]/mode <int>
/matrix/settings/sum_bus_master/[0-31]/mute <bool>
/matrix/settings/sum_bus_master/[0-31]/plugins/[0-3]/bypass <bool>
/matrix/settings/sum_bus_master/[0-31]/plugins/[0-3]/index <int>
/matrix/settings/sum_bus_master/[0-31]/plugins/[0-3]/type <int>
/matrix/settings/sum_bus_master/[0-31]/polarity <int>

/matrix/settings/video_in_base <int>

/matrix/settings/wck_io_status_tx/[0-1]/term_75 <bool>
/matrix/settings/wck_io_status_tx/[0-1]/wck_out_1FS <bool>

/matrix/settings/white_noise_generator/[0-1]/enabled <bool>
/matrix/settings/white_noise_generator/[0-1]/gain <float>
```

### Read-only OSC addresses (`STATE`, all but the `settings` subtree)

Note that the fact that all subtrees other than `settings` is made
read-only for OSC control is a deliberate decision of the audio server
implementation in order to not accidentally overwrite the system
configuration. This may be changed in the audio server
implementation. Nevertheless, some state values might not support
write access by the Prodigy matrix itself (e.g., the `device_info`
subtree).

```
/matrix/device_info/FPGA_FW_build/[0-1] <int>
/matrix/device_info/FPGA_FW_rev/[0-1] <int>
/matrix/device_info/cored_branch <string>
/matrix/device_info/cored_compiletime <string>
/matrix/device_info/cored_rev <string>
/matrix/device_info/cored_tag <string>
/matrix/device_info/front_panel_auto_lock <bool>
/matrix/device_info/front_panel_lock <bool>
/matrix/device_info/front_panel_lock_timeout <int>
/matrix/device_info/front_panel_pin_set <bool>
/matrix/device_info/image_build/[0-1] <string>
/matrix/device_info/model <string>
/matrix/device_info/name <string>
/matrix/device_info/shutdown_from_remote <bool>
/matrix/device_info/system_mode/description <string>
/matrix/device_info/system_mode/value <int>
/matrix/device_info/version_strings/[0-19] <string>

/matrix/display/display_brightness <int>
/matrix/display/led_dim <float>
/matrix/display/screensaver_timeout <float>

/matrix/fan/cpu_temp <float>
/matrix/fan/i12minus <float>
/matrix/fan/i12plus <float>
/matrix/fan/imain <float>
/matrix/fan/power <float>
/matrix/fan/psu/[0-1] <int>
/matrix/fan/silent <bool>
/matrix/fan/tach1_rpm <int>
/matrix/fan/tach2_rpm <int>
/matrix/fan/temp1 <float>
/matrix/fan/temp2 <float>
/matrix/fan/temp_local <float>
/matrix/fan/temp_thres <int>
/matrix/fan/v12minus <float>
/matrix/fan/v12plus <float>
/matrix/fan/v_psu/[0-1] <float>
/matrix/fan/vcc_intern <float>

/matrix/ip_config/address <string>
/matrix/ip_config/current_address <string>
/matrix/ip_config/current_gateway <string>
/matrix/ip_config/current_netmask <string>
/matrix/ip_config/dns <string>
/matrix/ip_config/gateway <string>
/matrix/ip_config/mac <string>
/matrix/ip_config/netmask <string>
/matrix/ip_config/use_dhcp <bool>

/matrix/last_snapshot_recalled <int>
/matrix/last_snapshot_recalled_name <string>
/matrix/last_snapshot_recalled_pos <int>

/matrix/licenses/automator/active <bool>
/matrix/licenses/automator/demo <bool>
/matrix/licenses/automator/demo_expired <bool>
/matrix/licenses/automator/running <bool>
/matrix/licenses/automator/valid <bool>
/matrix/licenses/automix <bool>
/matrix/licenses/bridge/active <bool>
/matrix/licenses/bridge/demo <bool>
/matrix/licenses/bridge/demo_expired <bool>
/matrix/licenses/bridge/running <bool>
/matrix/licenses/bridge/valid <bool>
/matrix/licenses/demo <bool>
/matrix/licenses/demo_expired <bool>
/matrix/licenses/ember/active <bool>
/matrix/licenses/ember/demo <bool>
/matrix/licenses/ember/demo_expired <bool>
/matrix/licenses/ember/running <bool>
/matrix/licenses/ember/valid <bool>
/matrix/licenses/loudness <bool>
/matrix/licenses/mix_sum/[0-3] <bool>
/matrix/licenses/osc/active <bool>
/matrix/licenses/osc/demo <bool>
/matrix/licenses/osc/demo_expired <bool>
/matrix/licenses/osc/running <bool>
/matrix/licenses/osc/valid <bool>
/matrix/licenses/plugin_flex/[0-3] <bool>
/matrix/licenses/snmp/active <bool>
/matrix/licenses/snmp/demo <bool>
/matrix/licenses/snmp/demo_expired <bool>
/matrix/licenses/snmp/running <bool>
/matrix/licenses/snmp/valid <bool>
/matrix/licenses/strings <string>
/matrix/licenses/system <int>

/matrix/mirroring/active <bool>
/matrix/mirroring/main_ip <string>
/matrix/mirroring/mirrors <string>
/matrix/mirroring/scope <string>
/matrix/mirroring/status <int>

/matrix/snapshots/[0-98]/author <string>
/matrix/snapshots/[0-98]/compliant <bool>
/matrix/snapshots/[0-98]/creation_timestamp <int>
/matrix/snapshots/[0-98]/dataset <string>
/matrix/snapshots/[0-98]/description <string>
/matrix/snapshots/[0-98]/locked <bool>
/matrix/snapshots/[0-98]/name <string>
/matrix/snapshots/[0-98]/position <int>
/matrix/snapshots/[0-98]/scopeset <string>
/matrix/snapshots/[0-98]/settings <string>
/matrix/snapshots/[0-98]/valid <bool>

/matrix/status/clock_source <int>
/matrix/status/ears_status/[0-5]/active <int>
/matrix/status/ears_status/[0-5]/audio_pilot <bool>
/matrix/status/ears_status/[0-5]/blds <bool>
/matrix/status/gpi/[0-1] <bool>
/matrix/status/input_manager/[0-31]/current <int>
/matrix/status/input_manager/[0-31]/signals/[0-5]/coherency <bool>
/matrix/status/input_manager/[0-31]/signals/[0-5]/silence <bool>
/matrix/status/input_manager/[0-31]/signals/[0-5]/trigger <bool>
/matrix/status/ltc_reader/drop_frame <int>
/matrix/status/ltc_reader/format <int>
/matrix/status/ltc_reader/frames <int>
/matrix/status/ltc_reader/hour <int>
/matrix/status/ltc_reader/lock <bool>
/matrix/status/ltc_reader/min <int>
/matrix/status/ltc_reader/multiplier <int>
/matrix/status/ltc_reader/sec <int>
/matrix/status/ltc_reader/stable <bool>
/matrix/status/ltc_reader/sync <bool>
/matrix/status/ltc_reader/valid <int>
/matrix/status/madi/[0-1]/lock <bool>
/matrix/status/madi/[0-1]/multiplier <int>
/matrix/status/madi/[0-1]/stable <bool>
/matrix/status/madi/[0-1]/sync <bool>
/matrix/status/madi_config/[0-1]/bit_transparent <bool>
/matrix/status/madi_config/[0-1]/ch_mode <int>
/matrix/status/madi_config/[0-1]/ch_mode_str <string>
/matrix/status/madi_config/[0-1]/frame_192k <bool>
/matrix/status/madi_config/[0-1]/frame_96k <bool>
/matrix/status/madi_config/[0-1]/src_in <bool>
/matrix/status/madi_config/[0-1]/src_out <bool>
/matrix/status/madi_netbase/[0-1]/fw_version <string>
/matrix/status/madi_netbase/[0-1]/redundancy_mode <int>
/matrix/status/madi_netbase/[0-1]/rx/[0-1]/ch_mode <int>
/matrix/status/madi_netbase/[0-1]/rx/[0-1]/frame_192k <bool>
/matrix/status/madi_netbase/[0-1]/rx/[0-1]/frame_96k <bool>
/matrix/status/madi_netbase/[0-1]/rx/[0-1]/lock <bool>
/matrix/status/madi_netbase/[0-1]/rx/[0-1]/redundancy_active <bool>
/matrix/status/madi_netbase/[0-1]/rx/[0-1]/src_fs <int>
/matrix/status/madi_netbase/[0-1]/rx/[0-1]/sync <bool>
/matrix/status/module_type/[0-11]/id <int>
/matrix/status/module_type/[0-11]/name <string>
/matrix/status/multiplier <int>
/matrix/status/net/[0-1]/lock <bool>
/matrix/status/net/[0-1]/multiplier <int>
/matrix/status/net/[0-1]/nics/0/dhcp <bool>
/matrix/status/net/[0-1]/nics/0/gateway <string>
/matrix/status/net/[0-1]/nics/0/ip <string>
/matrix/status/net/[0-1]/nics/0/netmask <string>
/matrix/status/net/[0-1]/nics/1/dhcp <bool>
/matrix/status/net/[0-1]/nics/1/gateway <string>
/matrix/status/net/[0-1]/nics/1/ip <string>
/matrix/status/net/[0-1]/nics/1/netmask <string>
/matrix/status/net/[0-1]/stable <bool>
/matrix/status/net/[0-1]/sync <bool>
/matrix/status/net_module_state/[0-1] <int>
/matrix/status/redundancy_state <int>
/matrix/status/ref_frequency <float>
/matrix/status/slot/[0-31]/lock <bool>
/matrix/status/slot/[0-31]/multiplier <int>
/matrix/status/slot/[0-31]/stable <bool>
/matrix/status/slot/[0-31]/sync <bool>
/matrix/status/video_in/cod <int>
/matrix/status/video_in/format <int>
/matrix/status/video_in/lock <bool>
/matrix/status/video_in/multiplier <int>
/matrix/status/video_in/stable <bool>
/matrix/status/video_in/sync <bool>
/matrix/status/warnings/active <bool>
/matrix/status/warnings/message <string>
/matrix/status/wck/[0-1]/lock <bool>
/matrix/status/wck/[0-1]/multiplier <int>
/matrix/status/wck/[0-1]/stable <bool>
/matrix/status/wck/[0-1]/sync <bool>

/matrix/triggers/actions/[0-3] <int>
```
