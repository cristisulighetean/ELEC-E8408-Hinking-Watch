# running web server in the background
sudo -u hikingHub python3 wserver.py &
PID_SERVER=$!

# running LED controller in the background
sudo python3 led.py &
PID_LED=$!

# running Receiver in the background
sudo -u hikingHub python3 receiver.py &
PID_RECEIVER=$!

# saving PIDs to file
echo "$PID_SERVER"$'\n'"$PID_LED"$'\n'"$PID_RECEIVER" > .bg_processes