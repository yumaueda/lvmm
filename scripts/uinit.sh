#!/bbin/gosh
echo '<4>SHELL_ALIVE' > /dev/kmsg
while true; do
    echo '<4>.' > /dev/kmsg
    /bbin/sleep 1
done
