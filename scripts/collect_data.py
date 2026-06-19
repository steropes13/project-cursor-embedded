import serial
import serial.tools.list_ports
import argparse
import sys
import threading

def select_serial_port():
    """
    Scans available COM/USB ports and asks the user to select one.
    """
    ports = list(serial.tools.list_ports.comports())
    if not ports:
        print("Error: No USB/Serial devices found.")
        sys.exit(1)

    print("\n--- Available Devices ---")
    for i, port in enumerate(ports):
        # Displays index, device name (e.g., COM3 or /dev/ttyUSB0) and description
        print(f"[{i}] {port.device} - {port.description}")

    while True:
        try:
            choice = int(input("\nEnter the number of the port to use: "))
            if 0 <= choice < len(ports):
                return ports[choice].device
            else:
                print("Invalid selection. Please choose a number from the list.")
        except ValueError:
            print("Invalid input. Please enter a valid integer.")

def main():
    # Setup CLI argument parsing
    parser = argparse.ArgumentParser(description="Serial Data Logger")
    parser.add_argument("filename", help="The name of the output file (e.g., data_log.txt)")
    args = parser.parse_args()

    # Get the port from the user
    port_name = select_serial_port()
    baud_rate = 9600  # Default baud rate (can be adjusted as needed)

    try:
        # Initialize serial connection
        # timeout=1 ensures the read loop doesn't hang indefinitely
        ser = serial.Serial(port_name, baud_rate, timeout=1)
        
        # Open file in append mode
        with open(args.filename, "w") as f:
            print(f"\n[CONNECTED] Port: {port_name} | Saving to: {args.filename}")
            print("[INFO] Press ENTER at any time to stop recording and save the file.\n")

            # Flag to signal the background thread to stop
            stop_event = threading.Event()

            def serial_reader_thread():
                """
                Continuously reads from serial, prints to console, and writes to file.
                """
                while not stop_event.is_set():
                    if ser.in_waiting > 0:
                        try:
                            # Read line and decode (using 'replace' to avoid crashes on noise)
                            raw_data = ser.readline().decode('utf-8', errors='replace').strip()
                            if raw_data:
                                print(raw_data)          # Echo to console
                                f.write(raw_data + "\n") # Write to file
                                f.flush()                # Ensure data is written to disk
                        except Exception as e:
                            print(f"\n[ERROR] During reading: {e}")
                            break

            # Start the background thread
            data_thread = threading.Thread(target=serial_reader_thread)
            data_thread.daemon = True # Thread dies when main process exits
            data_thread.start()

            # Wait for user input (any key/ENTER) to terminate
            input() 
            
            # Signal thread to stop and wait for it to finish
            stop_event.set()
            data_thread.join()

    except serial.SerialException as e:
        print(f"\n[SERIAL ERROR] Could not open port {port_name}: {e}")
    except KeyboardInterrupt:
        print("\n[INTERRUPTED] Process stopped by user.")
    finally:
        # Clean up: close serial port if it was opened
        if 'ser' in locals() and ser.is_open:
            ser.close()
        print(f"\n[FINISHED] Session closed. Data successfully saved in '{args.filename}'.")

if __name__ == "__main__":
    main()