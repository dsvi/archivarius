

config structure:

task "home backup"
{
	archive "/data/blah" 
	acl on ; or off 
	depth 6m
	sources{
		"My Documents"
		"My Pictures"
		"some-path/some-file"
	} 
}

