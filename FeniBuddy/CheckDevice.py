"""Check uploaded FeniBuddy over USB; do not change Wi-Fi or persistent settings."""
from __future__ import annotations
import argparse
import datetime as dt
import json
from pathlib import Path
import sys
import time
import urllib.error
import urllib.parse
import urllib.request

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / '.tools' / 'feni-python'))
import serial


def request(board, command, timeout=4):
    board.reset_input_buffer()
    board.write((command + '\n').encode('ascii'))
    board.flush()
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        line = board.readline().decode('utf-8', 'replace').strip()
        if command == 'STATUS' and line.startswith('{'):
            try:
                state = json.loads(line)
            except ValueError:
                continue
            if state.get('firmware') != 'feni-buddy-3.2.0':
                raise RuntimeError('Unexpected firmware: ' + str(state.get('firmware')))
            return state
        if command != 'STATUS' and line == 'OK':
            return line
    raise TimeoutError('No response to ' + command)


def summary(state):
    fields = ('firmware', 'input', 'touchEnabled', 'wifi', 'setup', 'ip', 'page', 'menu', 'choice', 'peek',
              'timerRunning', 'timerDone', 'timerSeconds', 'timerDurationSeconds',
              'customMinutes', 'customField', 'menuCount', 'faceFixed', 'eyeLeftX',
              'eyeRightX', 'eyeTargetY', 'touchReady',
              'touchActive', 'theme', 'meetingActive', 'meetingVisible', 'calendarFresh', 'heap', 'maxBlock', 'frames', 'uptimeMs', 'clock')
    return {key: state[key] for key in fields if key in state}


def smoke(board):
    passed = []

    def check(value, message):
        if not value:
            raise AssertionError(message)
        passed.append(message)
        print('PASS:', message, flush=True)

    status = lambda: request(board, 'STATUS')
    first = status()
    print('Initial:', json.dumps(summary(first)), flush=True)
    check(first['wifi'] or first['setup'], 'Wi-Fi or setup hotspot is active')
    time.sleep(0.5)
    current = status()
    check(current['uptimeMs'] > first['uptimeMs'], 'Firmware remains running')
    check(current['frames'] > first['frames'] + 2, 'Display frames advance')
    # Dismiss overlays and return to home without changing the display mode.
    for _ in range(5):
        request(board, 'BACK')
    current = status()
    check(current['page'] == 0, 'Back returns to the home screen')
    if not current['wifi'] or current['mode'] == 2:
        request(board, 'TAP')
        check(status()['peek'], 'Single tap opens temporary clock')
        request(board, 'BACK')
        check(not status()['peek'], 'Back dismisses temporary clock')
    check(current['menuCount'] == 5, 'Main menu has five entries')
    check(current['input'] == 'flash' and not current['touchEnabled'], 'Onboard FLASH is the active input')
    check(current['faceFixed'] and current['eyeLeftX'] == 36 and current['eyeRightX'] == 90
          and current['eyeTargetY'] == 45, 'Face stays centered without idle movement')

    opener = urllib.request.build_opener(urllib.request.ProxyHandler({}))
    base = 'http://' + first['ip']

    def http(path, values=None, token=None):
        body = None if values is None else urllib.parse.urlencode(values).encode()
        headers = {} if token is None else {'X-Feni-Token': token}
        req = urllib.request.Request(base + path, data=body, headers=headers)
        try:
            with opener.open(req, timeout=5) as response:
                return response.status, response.read()
        except urllib.error.HTTPError as error:
            return error.code, error.read()

    def capture(name):
        if not first['wifi']:
            return
        time.sleep(0.25)
        code, frame = http('/frame.bmp')
        check(code == 200 and frame[:2] == b'BM', name + ' framebuffer is available')
        (ROOT / 'runtime' / ('buddy-3.2-' + name + '.bmp')).write_bytes(frame)

    def home():
        for _ in range(5):
            request(board, 'BACK')
        check(status()['page'] == 0, 'Back returns to home')

    def menu_item(index):
        home()
        request(board, 'HOLD')
        current = status()
        check(current['page'] == 1, 'Hold opens the menu')
        for _ in range((index - current['menu']) % 5):
            request(board, 'TAP')
        if index == 4:
            capture('menu')
        request(board, 'HOLD')

    capture('face')
    menu_item(2)
    check(status()['page'] == 4, 'Timer menu opens')
    capture('timer-picker')
    if status()['timerRunning']:
        request(board, 'HOLD')
    for index, minutes in enumerate((10, 20, 30)):
        current = status()
        for _ in range((index - current['choice']) % 4):
            request(board, 'TAP')
        request(board, 'HOLD')
        current = status()
        check(current['timerRunning'] and current['timerDurationSeconds'] == minutes * 60
              and minutes * 60 - 2 <= current['timerSeconds'] <= minutes * 60,
              str(minutes) + '-minute preset starts with the correct duration')
        request(board, 'HOLD')
        check(not status()['timerRunning'], 'Hold cancels the running timer')
    request(board, 'TAP')
    request(board, 'HOLD')
    check(status()['page'] == 10, 'Custom timer editor opens')
    # Enter 001 using the same digit controls as the physical button.
    for field, place in enumerate((100, 10, 1)):
        current = status()
        digit = (current['customMinutes'] // place) % 10
        target = 1 if field == 2 else 0
        for _ in range((target - digit) % (2 if field == 0 else 10)):
            request(board, 'TAP')
        request(board, 'HOLD')
    current = status()
    check(current['customMinutes'] == 1 and current['customField'] == 3,
          'Custom digit editing selects one minute and advances to Start')
    capture('custom')
    request(board, 'HOLD')
    current = status()
    check(current['page'] == 4 and current['timerRunning'] and current['timerDurationSeconds'] == 60,
          'Hold starts the custom one-minute timer')
    timer_deadline = time.monotonic() + 65
    capture('countdown')
    request(board, 'BACK')
    check(status()['page'] == 1 and status()['timerRunning'], 'Timer continues after Back')

    menu_item(4)
    check(status()['page'] == 6, 'Settings menu opens')
    capture('settings')
    request(board, 'HOLD')
    check(status()['page'] == 11, 'Nested Wi-Fi menu opens')
    capture('wifi-menu')
    request(board, 'HOLD')
    check(status()['page'] == 7 and status()['ssid'] == first['ssid'], 'Wi-Fi details opens for the saved network')
    request(board, 'BACK')
    check(status()['page'] == 11 and status()['choice'] == 0, 'Wi-Fi details Back returns to Wi-Fi')
    request(board, 'TAP')
    request(board, 'HOLD')
    check(status()['page'] == 8, 'Show password opens on the physical display')
    if first['wifi']:
        code, _ = http('/frame.bmp')
        check(code == 403, 'Password screen cannot be downloaded as a framebuffer')
        check('password' not in status(), 'Status diagnostics do not contain the password')
    time.sleep(15.3)
    check(status()['page'] == 11 and status()['choice'] == 1, 'Password automatically hides after 15 seconds')
    request(board, 'TAP')
    request(board, 'HOLD')
    check(status()['page'] == 9 and status()['choice'] == 0, 'Forget Wi-Fi confirmation defaults to Cancel')
    capture('forget-confirm')
    request(board, 'TAP')
    request(board, 'BACK')
    check(status()['page'] == 11 and status()['ssid'] == first['ssid'], 'Back cancels Forget without changing Wi-Fi')
    request(board, 'HOLD')
    request(board, 'HOLD')
    check(status()['page'] == 11 and status()['ssid'] == first['ssid'], 'Holding Cancel preserves Wi-Fi')
    home()
    while time.monotonic() < timer_deadline:
        current = status()
        if current['timerDone']:
            break
        time.sleep(0.5)
    check(current['timerDone'] and not current['timerRunning'], 'Custom one-minute timer completes on the real board')
    capture('timer-done')
    request(board, 'BACK')
    check(status()['page'] == 0 and not status()['timerDone'], 'Back dismisses the timer alert to home')

    if first['wifi']:
        code, token = http('/session')
        check(code == 200, 'Local settings server is reachable')
        token = token.decode()
        check(http('/timer', {'minutes': 1})[0] == 403, 'Timer HTTP action requires a session token')
        for minutes in (0, 181, '1.5'):
            check(http('/timer', {'minutes': minutes}, token)[0] == 400,
                  'Timer HTTP input rejects ' + str(minutes))
        check(http('/timer', {'minutes': 25}, token)[0] == 200
              and status()['timerDurationSeconds'] == 1500, 'Browser starts a custom 25-minute timer')
        check(http('/timer', {'minutes': 1}, token)[0] == 409, 'A running timer is not replaced accidentally')
        check(http('/timer', {'action': 'cancel'}, token)[0] == 200
              and not status()['timerRunning'], 'Browser cancels the timer')
        home()
    current = status()
    check(current['uptimeMs'] > first['uptimeMs'], 'No reboot during device checks')
    check(current['ssid'] == first['ssid'] and current['mode'] == first['mode'], 'Saved network and display mode are unchanged')
    check(current['wifi'] == first['wifi'], 'Wi-Fi connection state is unchanged')
    print('Final:', json.dumps(summary(current)), flush=True)
    result = {'checkedAt': dt.datetime.now(dt.timezone.utc).isoformat(),
              'checks': passed, 'status': summary(current),
              'inputSource': 'serial and local HTTP commands; physical FLASH presses require a user check'}
    output = ROOT / 'runtime' / 'buddy-flash-check.json'
    output.write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', default='COM8')
    parser.add_argument('--watch', type=float, default=0, help='Observe touch/page changes for this many seconds; skip smoke tests')
    args = parser.parse_args()
    board = serial.Serial(port=None, baudrate=115200, timeout=0.15, write_timeout=2)
    board.port = args.port
    board.dtr = board.rts = False
    board.open()
    try:
        if args.watch:
            end = time.monotonic() + args.watch
            previous = None
            while time.monotonic() < end:
                state = request(board, 'STATUS')
                current = tuple(state.get(key) for key in ('touchReady', 'touchActive', 'page', 'menu', 'choice', 'peek'))
                if current != previous:
                    print(json.dumps(summary(state)), flush=True)
                    previous = current
                time.sleep(0.08)
        else:
            smoke(board)
    finally:
        board.close()


if __name__ == '__main__':
    main()
