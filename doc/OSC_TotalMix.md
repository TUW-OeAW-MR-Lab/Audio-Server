# TotalMix OSC Control

## TotalMix Settings
- Enable OSC Control via "Options" -> "Enable Osc Control"
- Go to OSC settings via "Options" -> "Settings" -> "OSC"
- Make sure that the incoming and outgoing ports match the settings in MRLabAudioServer's `Global.h`.
  - Default for incoming: 7001
  - Default for outgoing: 9001
- Enter "localhost" in "Remote Controller Address" -> "IP or HostName" (assuming that TotalMix and the MRLabAudioServer app are running on the same machine)
  - If you do not want to receive OSC messages from TotalMix, leave the field empty

## Loading snapshots via OSC commands
From the Webui (or any other OSC client of the MRLabAudioServer app), you can now send the following commands **to the MRLabAudioServer**. It will forward the commands to TotalMix internally.
Beware: You really load snapshot "1", by calling the command "../8/1". The numbering is inverted for some reason. These are not typos.

All commands are sent with a float payload of 1.0.

- load snapshot 1: `/totalmix/3/snapshots/8/1 1.0`
- load snapshot 2: `/totalmix/3/snapshots/7/1 1.0`
- load snapshot 3: `/totalmix/3/snapshots/6/1 1.0`
- load snapshot 4: `/totalmix/3/snapshots/5/1 1.0`
- load snapshot 5: `/totalmix/3/snapshots/4/1 1.0`
- load snapshot 6: `/totalmix/3/snapshots/3/1 1.0`
- load snapshot 7: `/totalmix/3/snapshots/2/1 1.0`
- load snapshot 8: `/totalmix/3/snapshots/1/1 1.0`
