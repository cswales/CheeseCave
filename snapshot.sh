#
# take a snapshot with the right arguments
# this is to the file snapshot.png
# at full resolution
# now (100ms from now)
# in PNG format
# without launching a preview window
#raspistill -o snapshot.png -w 1024 -h 768 -q 80 --nopreview -e png -t 100
#raspistill -o snapshot.jpg -e jpg -w 1024 -h 768 -q 80 --nopreview  -t 100
raspistill -o /opt/pi/CheeseCave/snapshot.jpg -e jpg -w 800 -h 600 -q 20 --nopreview  -t 100
cp /opt/pi/CheeseCave/snapshot.jpg /opt/pi/CheeseCave/current_snapshot
