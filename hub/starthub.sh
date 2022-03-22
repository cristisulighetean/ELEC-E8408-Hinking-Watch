sudo -u hikingHub python3 wserver.py &
PID_SERVER=$!
sudo python3 led.py &
PID_LED=$!
sudo -u hikingHub python3 receiver.py &
PID_RECEIVER=$!
echo "$PID_SERVER"$'\n'"$PID_LED"$'\n'"$PID_RECEIVER" > .bg_processes