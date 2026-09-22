const inputButtonIds = [
  'btn-input-select-DANTE_CAVEPC',
  'btn-input-select-DANTE_CurvedLEDPC',
  'btn-input-select-Curved_Wall_Bluetooth_Stereo',
  'btn-input-select-DANTE_Mobile'
];

export function init()
{
  enableInputButtons(true);
  clearActiveInputs();
}

function confirmDirectRouting(onConfirm)
{
  const dialog = document.getElementById('confirmDialog');
  if (!dialog) {
    onConfirm();
    return;
  }
  document.getElementById('confirmTitle').innerText = "Warning!";
  document.getElementById('confirmText').innerText = "This will route the outputs one-to-one, without any level changes to the CAVE speakers. Confirm that you have lowered the leves on your device to avoid speaker damage!";
  dialog.showModal();

  document.getElementById('okBtn').onclick = () => {
    dialog.close();
    onConfirm();
  };

  document.getElementById('cancelBtn').onclick = () => {
    dialog.close();
  };
}

export function setActiveInput(buttonId)
{
  inputButtonIds.forEach(id => {
    const btn = document.getElementById(id);
    if (btn) {
      if (id === buttonId) {
        btn.classList.add('active-input');
      } else {
        btn.classList.remove('active-input');
      }
    }
  });
}

export function clearActiveInputs()
{
  inputButtonIds.forEach(id => {
    const btn = document.getElementById(id);
    if (btn) btn.classList.remove('active-input');
  });
}

function enableInputButtons(enable = true)
{
  inputButtonIds.forEach(id => {
    const btn = document.getElementById(id);
    if (btn) btn.disabled = !enable;
  });
}

export function SetCavePc()
{
  confirmDirectRouting(() => {
    setActiveInput('btn-input-select-DANTE_CAVEPC');
    sendValue('/matrix/state/settings/easy_routing/400', 256);
    sendValue('/matrix/state/settings/easy_routing/401', 257);
    sendValue('/matrix/state/settings/easy_routing/402', 258);
    sendValue('/matrix/state/settings/easy_routing/403', 259);
    sendValue('/matrix/state/settings/easy_routing/404', 260);
    sendValue('/matrix/state/settings/easy_routing/405', 261);
    sendValue('/matrix/state/settings/easy_routing/406', 262);
    sendValue('/matrix/state/settings/easy_routing/407', 263);
  });
}

export function SetCurvedLedPc()
{
  confirmDirectRouting(() => {
    setActiveInput('btn-input-select-DANTE_CurvedLEDPC');
    sendValue('/matrix/state/settings/easy_routing/400', 128);
    sendValue('/matrix/state/settings/easy_routing/401', 129);
    sendValue('/matrix/state/settings/easy_routing/402', 130);
    sendValue('/matrix/state/settings/easy_routing/403', 131);
    sendValue('/matrix/state/settings/easy_routing/404', 132);
    sendValue('/matrix/state/settings/easy_routing/405', 133);
    sendValue('/matrix/state/settings/easy_routing/406', 134);
    sendValue('/matrix/state/settings/easy_routing/407', 135);
  });
}

export function SetCurvedBluetooth()
{
  confirmDirectRouting(() => {
    setActiveInput('btn-input-select-Curved_Wall_Bluetooth_Stereo');
    sendValue('/matrix/state/settings/easy_routing/400', 4136);
    sendValue('/matrix/state/settings/easy_routing/401', 4141);
    sendValue('/matrix/state/settings/easy_routing/402', 314); // BT L to FWL
    sendValue('/matrix/state/settings/easy_routing/403', 315); // BT R to FWR
    sendValue('/matrix/state/settings/easy_routing/404', 4156);
    sendValue('/matrix/state/settings/easy_routing/405', 4161);
    sendValue('/matrix/state/settings/easy_routing/406', 312); // BT Aux L to BL
    sendValue('/matrix/state/settings/easy_routing/407', 313); // BT Aux R to BR
  });
}

export function SetDanteMobile()
{
  confirmDirectRouting(() => {
    setActiveInput('btn-input-select-DANTE_Mobile');
    sendValue('/matrix/state/settings/easy_routing/400', 160);
    sendValue('/matrix/state/settings/easy_routing/401', 161);
    sendValue('/matrix/state/settings/easy_routing/402', 162);
    sendValue('/matrix/state/settings/easy_routing/403', 163);
    sendValue('/matrix/state/settings/easy_routing/404', 164);
    sendValue('/matrix/state/settings/easy_routing/405', 165);
    sendValue('/matrix/state/settings/easy_routing/406', 166);
    sendValue('/matrix/state/settings/easy_routing/407', 167);
  });
}

export function ResetDefaultRouting()
{
  sendNoArgs('/matrix/cmd/recall_snapshot_5');
  clearActiveInputs();
}
