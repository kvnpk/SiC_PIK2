from __future__ import print_function
import pyzbar.pyzbar as pyzbar
import numpy as np
import cv2
import time

# Function to decode QR/barcodes in an image
def decode(image):
    # Find barcodes and QR codes
    decoded_objects = pyzbar.decode(image)
    return decoded_objects

font = cv2.FONT_HERSHEY_SIMPLEX

# Get the webcam:  
cap = cv2.VideoCapture(1)
cap.set(3, 1280)  # Set width
cap.set(4, 720)   # Set height
time.sleep(2)

while cap.isOpened():
    # Capture frame-by-frame
    ret, frame = cap.read()
    if not ret:
        print("Failed to capture image")
        break

    # Decode QR/barcodes in the frame
    im_gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    decoded_objects = decode(im_gray)

    for decoded_object in decoded_objects:
        points = decoded_object.polygon
        # If the points do not form a quad, find convex hull
        if len(points) > 4:
            hull = cv2.convexHull(np.array([point for point in points], dtype=np.float32))
            hull = list(map(tuple, np.squeeze(hull)))
        else:
            hull = points

        # Number of points in the convex hull
        n = len(hull)
        # Draw the convex hull
        for j in range(n):
            cv2.line(frame, hull[j], hull[(j+1) % n], (255, 0, 0), 3)

        x = decoded_object.rect.left
        y = decoded_object.rect.top
        w = decoded_object.rect.width
        h = decoded_object.rect.height

        # Draw a bounding box around the QR code
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)

        # Overlay the decoded text
        barcode_data = decoded_object.data.decode("utf-8")
        barcode_type = decoded_object.type
        overlay_text = f'{barcode_type}: {barcode_data}'
        cv2.putText(frame, overlay_text, (x, y - 10), font, 0.5, (0, 255, 255), 2, cv2.LINE_AA)

    # Display the resulting frame
    cv2.imshow('Frame', frame)
    key = cv2.waitKey(1)
    if key & 0xFF == ord('q'):
        break
    elif key & 0xFF == ord('s'):  # Wait for 's' key to save 
        cv2.imwrite('Capture.png', frame)

# When everything is done, release the capture
cap.release()
cv2.destroyAllWindows()
