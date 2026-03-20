import cv2
import numpy as np

# --- Configuration ---
image_path = '3.png'  # <-- Replace with your actual HSV image file

# --- Load image (already in HSV format) ---
hsv_img = cv2.imread(image_path)

if hsv_img is None:
    print(f"Error: Could not load image '{image_path}'")
    exit()

# --- Mouse callback to show HSV values ---
def show_hsv(event, x, y, flags, param):
    if event == cv2.EVENT_MOUSEMOVE:
        h, s, v = hsv_img[y, x]
        print(f"HSV at ({x}, {y}) = H: {h}, S: {s}, V: {v}")

# --- Create window and bind mouse callback ---
cv2.namedWindow("HSV Image")
cv2.setMouseCallback("HSV Image", show_hsv)

# --- Show image and wait for ESC key ---
while True:
    cv2.imshow("HSV Image", hsv_img)
    if cv2.waitKey(1) & 0xFF == 27:  # Press ESC to exit
        break

cv2.destroyAllWindows()

