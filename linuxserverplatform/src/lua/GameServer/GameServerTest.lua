
g_isdubug = "扑鱼很简单";
g_table = {}
g_table.x = 1;
g_table.y = 20000;

function luotan(k)
	print("k="..k);
	return k;
end

function add(a, b)
	print("a="..a);
	print("b="..b);
	luotan(1234);
	return a + b;
end

function redis(command)
	return c_rediscmd(command);
end

function testredis()
	local table = redis("hgetall gameBaseInfo|11000100");
	
	--[[for k,v in pairs(table) do
		print(k,v);
	end--]]
	
	print("recv="..table);
	
	print("testredis");
end


function testload(param)
	
	print("val="..(param * 3));
	print("罗潭"..math.pow(param, 2));
	
	print(g_table.y)
end

function testreturn()
	return 1,2,3,4
end