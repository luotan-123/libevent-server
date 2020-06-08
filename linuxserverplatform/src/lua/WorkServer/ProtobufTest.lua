require 'person_pb'

local person= person_pb.ren()
person.id = 1000
person.name = "吴亦凡"
person.money = 16546

local laopo1 = person.list:add();
laopo1.name = "刘亦菲";
laopo1.age = 22

local laopo2 = person.list:add();
laopo2.name = "网红";
laopo2.age = 18


local data = person:SerializeToString()
local msg = person_pb.ren()
msg:ParseFromString(data)
print(msg)
