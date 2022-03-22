sudo -u hikingHub python3 wserver.py & disown
PID_SERVER=$!
sudo python3 led.py & disown
PID_LED=$!
sudo -u hikingHub python3 receiver.py & disown
PID_RECEIVER=$!
echo "$PID_SERVER"$'\n'"$PID_LED"$'\n'"$PID_RECEIVER" > .bg_processes