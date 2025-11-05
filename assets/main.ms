// main.ms
// Simple test script for MSRLWeb

print "main.ms running"

x = 100; y = 100
dx = 10; dy = 10

while true
	raylib.BeginDrawing
	
	raylib.ClearBackground raylib.BLACK
	raylib.DrawRectangle x, y, 200, 150, raylib.BLUE
	x += dx; y += dy
	if x + 200 > 960 and dx > 0 then dx = -dx
	if x < 0 and dx < 0 then dx = -dx
	if y + 150 > 640 and dy > 0 then dy = -dy
	if y < 0 and dy < 0 then dy = -dy
	
	raylib.EndDrawing
	yield
end while
