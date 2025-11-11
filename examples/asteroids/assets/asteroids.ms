import "listUtil"
import "mathUtil"
import "Sprite"

spriteSheetTex = raylib.LoadTexture("assets/Asteroids-2X.png")

print "Loading scoreFont..."
fontImage = raylib.LoadImage("assets/digits.png")
scoreFont = raylib.LoadFontFromImage(fontImage, [255, 0, 255], "0")
raylib.UnloadImage(fontImage)
print "scoreFont isFontValid: " + raylib.IsFontValid(scoreFont)
print scoreFont

sprites = []

zeroZero = {"x":0, "y":0}
score = 0

//----------------------------------------------------------------------

addPoints = function(points)
	outer.score += points
end function

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
GameSprite.srcTexture = spriteSheetTex
GameSprite.scale = 0.5
GameSprite.v = null  // x and y
GameSprite.destroyed = false
GameSprite.radius = null

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
	if self.radius == null and self.srcRect then
		self.radius = self.srcRect[2]/2 * self.scale
	end if
	sprites.push self
end function

GameSprite.destroy = function
	self.destroyed = true
	sprites.removeVal self
end function

GameSprite.overlaps = function(other)
	// For the sake of this demo, we'll assume all sprites are roughly
	// circular, so we can just do a distance test.
	r = self.radius + other.radius
	return (self.x - other.x)^2 + (self.y - other.y)^2 < r^2
end function

GameSprite.contains = function(point)
	// For the sake of this demo, we'll assume all sprites are roughly
	// circular, so we can just do a distance test.
	r = self.radius
	return (self.x - point.x)^2 + (self.y - point.y)^2 < r^2
end function

// Draw the radius circle, for debugging:
//GameSprite.draw = function
//	raylib.DrawCircle self.x, self.y, self.radius, [255, 255, 100, 128]
//	super.draw
//end function

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
ship.scale = 0.5
ship.radius = 18

ship.reset = function
	self.x = 480
	self.y = 320
	self.rotation = 0
end function

ship.init = function
	super.init
	self.srcRectEngineOff = [32*6, 32*8, 32*3, 32*2]
	self.srcRectEngineOn =  [32*9, 32*8, 32*3, 32*2]
	self.srcRect = self.srcRectEngineOff
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
Bullet.srcRect = [14*32, 9*32, 32, 32]
Bullet.radius = 2
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

Enemy = new GameSprite
Enemy.spin = 0
Enemy.scoreValue = 100

Enemy.update = function(dt)
	self.rotation = self.rotation + self.spin * dt
	if self.overlaps(ship) and not ship.destroyed then
		ship.destroy	// comment out this line for invulnerability!
		self.destroy
	else
		for b in Bullet.instances
			if self.contains(b) then
				addPoints self.scoreValue
				self.destroy
				b.destroy
			end if
		end for	
	end if
	super.update dt	
end function

//----------------------------------------------------------------------

Rock = new Enemy
Rock.scale = 0.5
Rock.size = 0
Rock.srcRects = []  // indexed by type; within each type, indexed by size
for type in range(0, 2)
	Rock.srcRects.push []
	Rock.srcRects[type].push [160*type, 0, 160, 160]
	Rock.srcRects[type].push [96*type, 160, 96, 96]
	Rock.srcRects[type].push [64*type, 256, 64, 64]
end for

Rock.init = function(size=0)
	super.init
	self.size = size
	self.srcRect = Rock.srcRects[rnd*3][size]
	self.radius = self.srcRect[2]/2 * 0.75 * self.scale
	self.spin = 360 * (rnd - 0.5)
end function

Rock.scoreValue = function
	return [20, 50, 100][self.size]
end function

Rock.destroy = function
	super.destroy
	hit.play 0.7, (self.x - 480)/480, 0.5 + 0.2*self.size
	makeDebris self
	if self.size < 2 then
		// break into two rocks of the next size down
		Rock.Make self.size+1, self
		Rock.Make self.size+1, self
	end if
	//heartbeat.updateInterval
end function

Rock.Make = function(size=0, position=null)
	rock = new Rock
	rock.init size
	if position then
		rock.x = position.x
		rock.y = position.y
	else
		rock.x = 960 * rnd
		rock.y = 640 * rnd
	end if
	rock.v.x = (rnd-0.5) * 300
	rock.v.y = (rnd-0.5) * 300
end function

//----------------------------------------------------------------------

makeDebris = function(source)
	for i in range(10)
		d = new TimedSprite
		d.init
		d.srcRect = [14*32, 9*32, 32, 32]
		d.x = source.x; d.y = source.y; d.tint = source.tint
		d.v.x = (rnd-0.5) * 300
		d.v.y = (rnd-0.5) * 300		
	end for
end function

//----------------------------------------------------------------------

//----------------------------------------------------------------------


init = function
	ship.init
	for i in range(0,7)
		Rock.Make
	end for
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

	for sp in sprites
		sp.draw
	end for
	
	scoreStr = ("00000" + score)[-5:]
	raylib.DrawTextEx scoreFont, scoreStr, [780, 0], scoreFont.baseSize, 2, color.lime
	
	raylib.DrawFPS
	raylib.EndDrawing
end function

run = function
	init
	while true
		update
		render
		if raylib.IsKeyPressed(raylib.KEY_ESCAPE) or raylib.IsKeyPressed(raylib.KEY_Q) then break
		yield
	end while	
end function

if locals == globals then run
