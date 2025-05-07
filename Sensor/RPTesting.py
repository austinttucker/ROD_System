import RPi.GPIO as GPIO
import time
import boto3

rgbPins = {'Red':18, 'Green':27, 'Blue':22}
pirPin = 17    # the PIR sensor connected to pin 17

# Time interval in seconds
CHECK_INTERVAL = 300 

motion_detected = False  # Shared variable to track motion status

access_key = 'AKIAXKPUZQEXHPC64HBJ'
secret_access_key = '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'
# Initialize DynamoDB client
dynamodb = boto3.resource('dynamodb', aws_access_key_id=access_key, aws_secret_access_key=secret_access_key, region_name='us-east-1')
 
# Your table name
table = dynamodb.Table('Room_Occupancy_Table')


def setup():
    global p_R, p_G, p_B
    GPIO.setmode(GPIO.BCM)      # Set the GPIO modes to BCM Numbering
    GPIO.setup(pirPin, GPIO.IN)    # Set pirPin to input
    # Set all LedPin's mode to output and initial level to High (3.3v)
    for i in rgbPins:
        GPIO.setup(rgbPins[i], GPIO.OUT, initial=GPIO.HIGH)

    # Set all LED pins as PWM channel and frequency to 2KHz
    p_R = GPIO.PWM(rgbPins['Red'], 2000)
    p_G = GPIO.PWM(rgbPins['Green'], 2000)
    p_B = GPIO.PWM(rgbPins['Blue'], 2000)

    # Initialize with value 0
    p_R.start(0)
    p_G.start(0)
    p_B.start(0)

    # Set up the interrupt for the PIR sensor (rising edge detection)
    GPIO.add_event_detect(pirPin, GPIO.RISING, callback=motion_detected_callback)

# Define a MAP function for mapping values (e.g., from 0~255 to 0~100)
def MAP(x, in_min, in_max, out_min, out_max):
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min

# Define a function to set up colors
def setColor(color):
    R_val = (color & 0xFF0000) >> 16
    G_val = (color & 0x00FF00) >> 8
    B_val = (color & 0x0000FF) >> 0
    # Map color values from 0~255 to 0~100
    R_val = MAP(R_val, 0, 255, 0, 100)
    G_val = MAP(G_val, 0, 255, 0, 100)
    B_val = MAP(B_val, 0, 255, 0, 100)

    # Assign the mapped duty cycle value to the corresponding PWM channel
    p_R.ChangeDutyCycle(R_val)
    p_G.ChangeDutyCycle(G_val)
    p_B.ChangeDutyCycle(B_val)

def motion_detected_callback(channel):
    global motion_detected
    motion_detected = True  # Motion detected, set the flag

def loop():
    global motion_detected
    while True:
        # If motion is detected, act on it
        if motion_detected:
            update_dynamodb(1000000, True)
            motion_detected = False  # Reset the motion detected flag

        # Wait for the next check interval
        time.sleep(CHECK_INTERVAL)

def destroy():
    p_R.stop()
    p_G.stop()
    p_B.stop()
    GPIO.cleanup()  # Release GPIO resources


def update_dynamodb(room_number, is_occupied):
    response = table.put_item(
        Item={
            'room_number': room_number,                           # Partition key
            'is_occupied': is_occupied                             # Room status
        }
    )
    print("PutItem succeeded:", response)


if __name__ == '__main__':
    setup()

    try:
        loop()  # Main loop to process the PIR sensor data every time interval
    except KeyboardInterrupt:  # Gracefully handle Ctrl+C
        destroy()
