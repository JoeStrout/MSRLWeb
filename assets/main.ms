// main.ms
// Simple test script for MSRLWeb

print "main.ms running"

x = 100; y = 100
dx = 10; dy = 10

while true
	raylib.BeginDrawing
	
	raylib.ClearBackground raylib.BLACK
	raylib.DrawRectangle x, y, 200, 150, raylib.BLUE

	if raylib.IsKeyDown(raylib.KEY_LEFT) then x -= dx
	if raylib.IsKeyDown(raylib.KEY_RIGHT) then x += dx
	if raylib.IsKeyDown(raylib.KEY_UP) then y -= dy
	if raylib.IsKeyDown(raylib.KEY_DOWN) then y += dy

	if raylib.IsMouseButtonPressed(raylib.MOUSE_BUTTON_LEFT) then
		pos = raylib.GetMousePosition
		print "Clicked at " + pos.x + ", " + pos.y
		x = pos.x
		y = pos.y
	end if
	
	raylib.EndDrawing
	yield
end while
