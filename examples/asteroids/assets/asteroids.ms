import "listUtil"
import "mathUtil"
import "Sprite"

spriteSheetTex = raylib.LoadTexture("assets/Asteroids-2X.png")

sprites = []

zeroZero = {"x":0, "y":0}

//----------------------------------------------------------------------

key = {}
key.axis = function(which)
	if which == "Horizontal" then
		return raylib.IsKeyDown(raylib.KEY_RIGHT) +
		       raylib.IsKeyDown(raylib.KEY_D) -
		       raylib.IsKeyDown(raylib.KEY_LEFT) - 
		       raylib.IsKeyDown(raylib.KEY_A)
	else if which == "Vertical" then
		// Note: flipped per Raylib's top-down coordinate system
		return raylib.IsKeyDown(raylib.KEY_DOWN) +
		       raylib.IsKeyDown(raylib.KEY_S) -
		       raylib.IsKeyDown(raylib.KEY_UP) - 
		       raylib.IsKeyDown(raylib.KEY_W)
	end if
end function
	 
//----------------------------------------------------------------------

Sound = {}
Sound.play = function(volume=1, pan=0, speed=1)
	// ToDo
end function
Sound.stop = function
	// ToDo
end function

pew = new Sound
bipBoop = [new Sound, new Sound]
hit = new Sound
engineNoise = new Sound
ufoSound = new Sound

//----------------------------------------------------------------------

color = {}
color.lime = [0, 255, 0, 255]

//----------------------------------------------------------------------

Bounds = {}
Bounds.x = 0; Bounds.y = 0
Bounds.width = 100; Bounds.height = 100

//----------------------------------------------------------------------

GameSprite = new Sprite
GameSprite.scale = 0.5
GameSprite.v = null  // x and y
GameSprite.destroyed = false

GameSprite.update = function(dt)
	self.x = self.x + self.v.x * dt
	if self.x > 1040 then
		self.x = -80
	else if self.x < -80 then
		self.x = 1040
	end if
	self.y = self.y + self.v.y * dt
	if self.y > 720 then
		self.y = -80
	else if self.y < -80 then
		self.y = 720
	end if
end function

GameSprite.init = function
	self.v = {"x":0, "y":0}
	self.destroyed = false
	sprites.push self
end function

GameSprite.destroy = function
	self.destroyed = true
	sprites.removeVal self
end function

//----------------------------------------------------------------------

ship = new GameSprite
ship.acceleration = 2000  // pixels/sec/sec
ship.deceleration = 100   // pixels/sec/sec
ship.maxSpeed = 1200 // pixels/sec
ship.turnRate = 720  // degrees/sec
ship.fireWasPressed = false
ship.localBounds = new Bounds
ship.localBounds.width = 64
ship.localBounds.height = 32
ship.tint = color.lime
ship.thrusting = false

ship.reset = function
	self.x = 480
	self.y = 320
	self.rotation = 0
end function

ship.init = function
	super.init
	self.srcTexture = spriteSheetTex
	self.srcRectEngineOff = [32*6, 32*8, 32*3, 32*2]
	self.srcRectEngineOn =  [32*9, 32*8, 32*3, 32*2]
	self.srcRect = self.srcRectEngineOff
	self.scale = 0.5
	self.reset
end function

ship.update = function(dt)
	// turn
	turn = key.axis("Horizontal")
	self.rotation = self.rotation + turn * self.turnRate * dt
		
	// thrust
	thrust = key.axis("Vertical") or raylib.IsKeyDown(raylib.KEY_LEFT_SHIFT) or
	  raylib.IsKeyDown(raylib.KEY_RIGHT_SHIFT)
	if thrust < 0 then thrust = 0
	radians = self.rotation * pi/180
	self.v.x = self.v.x + cos(radians) * thrust * self.acceleration * dt
	self.v.y = self.v.y + sin(radians) * thrust * self.acceleration * dt
	if thrust > 0 and not self.thrusting then
		self.srcRect = self.srcRectEngineOn
		engineNoise.play 0.1
		self.thrusting = true
	else if self.thrusting and thrust == 0 then
		self.srcRect = self.srcRectEngineOff
		engineNoise.stop
		self.thrusting = false
	end if
	
	// fire bullets
	fireIsPressed = raylib.IsKeyDown(raylib.KEY_SPACE)
	if fireIsPressed and not self.fireWasPressed then
		b = new Bullet
		b.init
		b.tint = self.tint
		b.x = self.x + cos(radians)*16
		b.y = self.y + sin(radians)*16
		b.v.x = self.v.x + cos(radians)*500
		b.v.y = self.v.y + sin(radians)*500
		pew.play 1, (self.x - 480)/480
	end if
	self.fireWasPressed = fireIsPressed
	
	// apply friction and max speed
	mathUtil.moveTowardsXY self.v, zeroZero, self.deceleration * dt
	speed = sqrt(self.v.x^2 + self.v.y^2)
	if speed > self.maxSpeed then
		f = self.maxSpeed / speed
		self.v.x = self.v.x * f
		self.v.y = self.v.y * f
	end if
	
	super.update dt
end function

//----------------------------------------------------------------------

TimedSprite = new GameSprite
TimedSprite.despawnTime = 0

TimedSprite.init = function(duration = 0.5)
	super.init
	self.despawnTime = time + duration
end function

TimedSprite.update = function(dt)
	if time > self.despawnTime then
		self.destroy
	else
		super.update dt
	end if
end function

//----------------------------------------------------------------------

Bullet = new TimedSprite
Bullet.srcTexture = spriteSheetTex
Bullet.srcRect = [14*32, 9*32, 32, 32]
Bullet.instances = []

Bullet.init = function(lifetime = 0.6)
	super.init lifetime
	Bullet.instances.push self
end function

Bullet.destroy = function
	super.destroy
	Bullet.instances.removeVal self
end function

//----------------------------------------------------------------------

init = function
	ship.init
end function

update = function(dt)
	if dt == null then dt = raylib.GetFrameTime()
	for sp in sprites
		sp.update dt
	end for
end function

render = function
	raylib.BeginDrawing
	raylib.ClearBackground raylib.BLACK

	//raylib.DrawTexture spriteSheetTex //, x, y, raylib.WHITE
	for sp in sprites
		sp.draw
	end for
	
	raylib.DrawFPS
	raylib.EndDrawing
end function

run = function
	init
	while true
		update
		render
		yield
	end while	
end function

if locals == globals then run
