package Common is    -- untested...
   type STATE_TYPE is (init, left, right, animation, screen_saver);
   function GETID (x,y,rows,cols : integer) return integer;
end Common;

package body Common is
	function GETID (x, y, rows, cols : integer) return integer is
	    variable real_x, real_y : integer;
	begin
	    real_x := x mod cols;
	    real_y := y mod rows;
	    return real_y * cols + real_x;
	end GETID;
   -- subprogram bodies here
end Common;