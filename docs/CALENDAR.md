# Connect Google Calendar

Feni reads a small feed from a Google Apps Script owned by you. The script runs
in Google's service, so your computer can be off. It only needs Calendar read
permission; Google OAuth credentials are never copied to the ESP8266.

1. Open [Google Apps Script](https://script.google.com/) and create a project
   named **Feni Calendar Bridge**.
2. Replace `Code.gs` with [the supplied script](../FeniBuddy/calendar/Code.gs).
3. Open Project settings and enable **Show appsscript.json manifest file**.
4. In the editor, replace that manifest with
   [appsscript.json](../FeniBuddy/calendar/appsscript.json). This explicitly limits
   the scope to `calendar.readonly`.
5. Save, select `setupFeni`, and Run. Authorize the script in your own Google
   account. If Google displays an unverified-app notice for your personal script,
   verify that you own the project and review its read-only Calendar permission.
6. In Project settings / Script properties, copy the generated **FENI_KEY**.
   Keep it private. **CALENDAR_ID** defaults to your primary Calendar; change it
   here if you want a different calendar you can read.
7. Choose **Deploy / New deployment / Web app**. Set **Execute as: Me** and
   **Who has access: Anyone**, then deploy. The script requires FENI_KEY before
   returning any event data; a request without it returns only `{"ok":false}`.
8. Copy the deployed URL ending in `/exec`, not the project editor URL or `/dev`.
9. Open Feni's local web page, expand **Connect or change calendar**, enter the
   deployment URL and FENI_KEY, and select **Connect calendar**.
10. Return Feni to Home and wait for **Calendar synced**. Its clock must be set
    for HTTPS certificate checks. Use **Refresh events** to request another sync.

The feed contains up to eight current/upcoming events from the next 14 days.
Recurring instances are expanded; declined and ended events are omitted. Titles
are converted to printable ASCII for the display. Timed events have reminders
ten minutes before and at their start. All-day entries do not trigger a midnight
reminder. Stale cached data is marked and does not generate new reminders.

Feni normally checks every two minutes while at Home or on the running-timer
screen. A long menu session or loss of internet can delay updates. The cached
events disappear on reboot and are fetched again after the connection returns.

## Updating the bridge

Replace the code as needed, then use **Deploy / Manage deployments / Edit /
New version / Deploy** to update the existing deployment. Merely saving the
editor does not update `/exec`. Reusing the deployment retains Feni's URL and key.

## Troubleshooting

- **Waiting for clock sync:** check internet access or use the local time-sync control.
- **Deployment needs access:** verify Execute as Me and access Anyone.
- **Check deployment / key:** use the correct `/exec` URL and complete FENI_KEY.
- **Network/TLS error:** check internet access, device time and the documented MMU setting.
- Workspace administrators may prohibit anonymous web-app deployments. Use an
  account that permits this setup; the firmware cannot override that restriction.

Never commit your key, deployment-specific configuration, or flash backups.
Use **Disconnect calendar** in Feni's web page to remove its connection details.
