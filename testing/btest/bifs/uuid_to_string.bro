#
# @TEST-EXEC: bro %INPUT >out
# @TEST-EXEC: btest-diff out

event bro_init()
	{
	local a = "\xfe\x80abcdefg0123456";
	print uuid_to_string(a);
	print uuid_to_string("");
	}
