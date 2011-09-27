-- Ui for drawing the classic HippoPlayer inteface

-- create is called with starting the app (or (re)loading the skin)

function create()
	print("create")
end

-- update is called each time the UI needs to be (re)drawn 

play_file = "testing"

function update()

	-- clear the whole area
	hippo_ui.fill(0x00a0a0a0, 0, 0, 264, 136)

	-- draw border around the info area
	hippo_ui.drawBorder(0x0000000, 0x00ffffff, 2, 14, 256, 20)
	hippo_ui.drawBorder(0x00ffffff, 0x00000000, 25, 65, 224, 67)
	hippo_ui.fill(0x00aaaaaa, 26, 66, 157, 67)

	--
	--hippo_ui.beginVerticalStackPanelXY(10, 24)
	--  hippo_ui.textLabel("HippoPlayer X 0.1");
	 -- hippo_ui.textLabel("Coded by Daniel Collin");

	-- top row of buttons
	hippo_ui.beginHorizontalStackPanelXY(0, 0)
	  if hippo_ui.buttonImage("skins/classic/close.png") then
	  	hippo.quit()
	  end
	  hippo_ui.staticImage("skins/classic/upper_bar.png")
	  hippo_ui.staticImage("skins/classic/hide.png")
	  hippo_ui.staticImage("skins/classic/minimize.png")
	
	-- buttons for playing/stop/open/etc
	hippo_ui.beginHorizontalStackPanelXY(0, 38)
	  if hippo_ui.buttonImage("skins/classic/play.png") then
		print(play_file)
	  	hippo.playFile(play_file)
	  end

	  if hippo_ui.buttonImage("skins/classic/open.png") then
		hippo_ui.fileDialogPlaylist()
	  end

	files = hippo.hasPlaylistFiles()

	-- todo: Add support for staticImage with coords
	hippo_ui.beginHorizontalStackPanelXY(158, 66)
		if files == nil then
			hippo_ui.staticImage("skins/classic/hippo.png")
		end

	-- Do the listbox here
	hippo_ui.beginVerticalStackPanelXY(20, 70)

	local pos = hippo_ui.slider()
	local file = hippo_ui.playList(25, 70, 100, 50, pos)

	if file then
 		play_file = file
 	end

	-- hippo_ui.listBoxXY(25, 56, 244, 64, t)

end

-- destroy is called before closing down the app or switching skin

function destroy()
	print("destroy")
end