from PIL import Image
import numpy as np

def validate_pixel_connections(image_path, steps_path):
    # 1. Load Image and Find All Necessary Connections
    img = Image.open(image_path).convert("L")
    grid = np.array(img) < 128  # True if black (ink)
    height, width = grid.shape
    
    # We define a connection as a tuple of two adjacent (x,y) points
    # We use frozenset to make the order (A->B or B->A) irrelevant
    required_connections = set()
    
    for y in range(height):
        for x in range(width):
            if grid[y][x]:
                # Check Right neighbor
                if x + 1 < width and grid[y][x+1]:
                    required_connections.add(frozenset([(x, y), (x + 1, y)]))
                # Check Down neighbor
                if y + 1 < height and grid[y+1][x]:
                    required_connections.add(frozenset([(x, y), (x, y + 1)]))

    # 2. Simulate the Steps and Record Traversed Connections
    traversed_connections = set()
    move_map = {"UP": (0, -1), "DOWN": (0, 1), "LEFT": (-1, 0), "RIGHT": (1, 0)}
    
    curr_x, curr_y = 0, 0 # Assume start at Top-Left
    
    with open(steps_path, "r") as f:
        for line in f:
            line = line.strip()
            if not line or "END" in line.upper(): continue
            
            direction, dist = line.split()[0].upper(), int(line.split()[1])
            dx, dy = move_map[direction]
            
            for _ in range(dist):
                prev_pos = (curr_x, curr_y)
                curr_x += dx
                curr_y += dy
                new_pos = (curr_x, curr_y)
                
                # Record this move as a traversed connection
                traversed_connections.add(frozenset([prev_pos, new_pos]))

    # 3. Final Analysis
    missed_connections = required_connections - traversed_connections
    
    total = len(required_connections)
    made = len(required_connections & traversed_connections)
    coverage = (made / total) * 100 if total > 0 else 100
    
    print(f"--- CONNECTION ANALYSIS ---")
    print(f"Total Unique Adjacencies in Bitmap: {total}")
    print(f"Adjacencies Traversed by Path:     {made}")
    print(f"Connection Coverage:               {coverage:.2f}%")
    
    if missed_connections:
        print(f"\nFAILED: {len(missed_connections)} connections were skipped.")
        print("Example missed connection (first 3):")
        for i, conn in enumerate(list(missed_connections)[:3]):
            pts = list(conn)
            print(f"  - Between {pts[0]} and {pts[1]}")
    else:
        print("\nSUCCESS: Every adjacent pixel connection in the image was traversed!")

# Execute
validate_pixel_connections("output/tmp/fully_connected_image.bmp", "output/tmp/human_steps.txt")