import network
import ujson
import time
from ws import AsyncWebsocketClient
import uasyncio
import ubinascii as binascii
import machine
from machine import Pin

ws = None

async def setup():
    global ws
    wifi = network.WLAN(network.STA_IF)
    wifi.active(1)
    wifi.connect("Wokwi-GUEST","")
    print("Connecting to WiFi", end="")
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect("Wokwi-GUEST", "")
    while not wlan.isconnected():
        print(".", end="")
        time.sleep(0.1)
    print(" Connected!")
    print(wlan.ifconfig())
    ws = AsyncWebsocketClient()
    # Initialize the LED state
    current_state = "free"
    print("Doing handshake")
    await ws.handshake("wss://parking-rio.rezel.net/api")
    print("Handshake done <3")



echo_pin = Pin(2, Pin.IN)
trig_pin = Pin(4, Pin.OUT)
led_red_pin = Pin(8, Pin.OUT)
led_green_pin = Pin(7, Pin.OUT)
led_blue_pin = Pin(6, Pin.OUT)

last_time = 0
timer_delay = 5000
idPlace = -1
sound_speed = 343.2
previous_state = False
current_state = False


def get_mac_address():
    return binascii.hexlify(network.WLAN().config('mac'), ':').decode('utf-8')



def send_data_via_websocket(data):
    if idPlace != -1 :
        data["id"] = idPlace
    data["name"] = get_mac_address()
    stringed = ujson.dumps(data)
    async def send():
        global ws
        try:
            await ws.send(stringed)
            print("request sent :" + stringed)
        except Exception as e:
            print(f"Error sending data via WebSocket: {str(e)}")
    uasyncio.run(send())




# WebSocket handler function
async def websocket_handler():
    global current_state
    global ws
    try:
        while True:
            message = await ws.recv()
            if message:
                print("Received data:", message)

                # Deserialize JSON
                data = ujson.loads(message)
                if "request" in data:
                    request = data["request"]
                    if request == "name":
                        response = { "response": "name"}
                        if idPlace == -1 :
                            request = { "request": "getId"}
                            send_data_via_websocket(request)
                        send_data_via_websocket(response)
                    elif request == "setId" :
                        idPlace = data["id"]
                    elif request == "state":
                        response = {
                            "response":"state"
                        }
                        if currentState is not None:
                            if currentState == "busy":
                                response["state"] = "busy"
                            elif currentState == "free":
                                response["state"] = "free"
                            elif currentState == "reserved":
                                response["state"] = "reserved"
                            else:
                                response["state"] = "error"
                        send_data_via_websocket(response)

                    elif request == "setState":
                        response = {
                            "response":"setState",
                            "state":"reserved"
                        }
                        send_data_via_websocket(response)
                        current_state = "reserved"
                        previous_state = "reserved"
                    else:
                        # we do nothing
                        pass
                elif "response" in data :
                    response = data["response"]
                    if response == "getId":
                        idPlace == data["id"]
            else:
                return
    except Exception as e:
        print("Error sending data via WebSocket: {"+str(e)+"}")


def switch_led(color):
    led_red_pin.off() # Turn off red LED
    led_green_pin.off()
    led_blue_pin.off()
    if color == "red":
        led_red_pin.on()  # Set maximum intensity for red LED
    elif color == "green":
        led_green_pin.on()  # Set maximum intensity for green LED
    elif color == "blue":
        led_blue_pin.on()


def check_sensor():
    global last_time, current_state, previous_state

    trig_pin.off()
    time.sleep_us(2)
    trig_pin.on()
    time.sleep_us(10)
    trig_pin.off()

    duration = machine.time_pulse_us(echo_pin, 1, 30000)  # 30ms timeout for 5m distance
    distance = duration * (sound_speed / (2 * 10 ** 4))
    disp = str(distance)

    print("Distance: " + disp + " cm")

    if distance <= 50 :
        current_state = "busy"
    else: 
        if previous_state != "reserved":
            current_state = "free"
        else:
            current_state = "reserved"

    if current_state != previous_state :
        print("state changed, current state is :"+current_state)
        if current_state == "busy" :
            switch_led("red")
        elif current_state == "reserved" :
            switch_led("blue")
        else:
            switch_led("green")

        send_data_via_websocket({"info": "state", "state": current_state})    
        previous_state = current_state


def main():
    uasyncio.run(setup())
    while True:
        check_sensor()
        uasyncio.run(websocket_handler())
        print("Now sleeping")
        time.sleep(5)


if __name__ == "__main__":
    main()
