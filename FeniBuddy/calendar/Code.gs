/** Feni Calendar bridge. Read-only; no event contents beyond title and times. */
function setupFeni() {
  const properties = PropertiesService.getScriptProperties();
  if (!properties.getProperty('FENI_KEY')) {
    properties.setProperty('FENI_KEY', Utilities.getUuid().replace(/-/g, '') + Utilities.getUuid().replace(/-/g, ''));
  }
  if (!properties.getProperty('CALENDAR_ID')) {
    properties.setProperty('CALENDAR_ID', CalendarApp.getDefaultCalendar().getId());
  }
  // Force the initial authorization check; do not write the key into execution logs.
  CalendarApp.getCalendarById(properties.getProperty('CALENDAR_ID')).getName();
  console.log('Ready. Copy FENI_KEY from Project Settings > Script properties. Deploy as a web app.');
}

function constantTimeEqual(left, right) {
  let different = left.length ^ right.length;
  for (let i = 0; i < Math.max(left.length, right.length); ++i) {
    different |= (left.charCodeAt(i) || 0) ^ (right.charCodeAt(i) || 0);
  }
  return different === 0;
}

function compactEvents(source, nowMs) {
  const result = new Map();
  for (const event of source) {
    const startMs = event.getStartTime().getTime();
    const endMs = event.getEndTime().getTime();
    if (endMs <= nowMs || endMs <= startMs || startMs >= nowMs + 14 * 86400000) continue;
    if (event.getMyStatus() === CalendarApp.GuestStatus.NO) continue;
    const start = Math.floor(startMs / 1000);
    // Recurrence instances share Google IDs. Include the occurrence's start time.
    const hash = Utilities.computeDigest(Utilities.DigestAlgorithm.SHA_256, event.getId() + ':' + start);
    const id = hash.map(value => ((value + 256) % 256).toString(16).padStart(2, '0')).join('').slice(0, 24);
    const title = event.getTitle().normalize('NFKD').replace(/[^\x20-\x7e]/g, '').replace(/\s+/g, ' ').trim().slice(0, 72) || 'Calendar event';
    result.set(id, {id, title, start, end: Math.floor(endMs / 1000), allDay: event.isAllDayEvent()});
  }
  return [...result.values()].sort((a, b) => a.start - b.start || a.id.localeCompare(b.id)).slice(0, 8);
}

function doGet(request) {
  const respond = value => ContentService.createTextOutput(JSON.stringify(value)).setMimeType(ContentService.MimeType.JSON);
  const properties = PropertiesService.getScriptProperties();
  const expected = properties.getProperty('FENI_KEY') || '';
  const supplied = String(request && request.parameter && request.parameter.key || '');
  if (expected.length < 32 || supplied.length > 64 || !constantTimeEqual(expected, supplied)) return respond({ok: false});
  try {
    const calendar = CalendarApp.getCalendarById(properties.getProperty('CALENDAR_ID'));
    if (!calendar) return respond({ok: false});
    const nowMs = Date.now();
    // getEvents expands recurring instances, including edits and cancellations.
    const events = compactEvents(calendar.getEvents(new Date(nowMs), new Date(nowMs + 14 * 86400000)), nowMs);
    return respond({ok: true, fetchedAt: Math.floor(nowMs / 1000), events});
  } catch (error) {
    return respond({ok: false});
  }
}
