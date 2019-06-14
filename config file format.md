# Config file format

Example:
 
	task home folder backup {
		archive /media/me/backup/
		max-storage-time 6m
		root /home/me/
		exclude {
			.cache
			.steam
			Steam
			Downloads
		}
		password ma qwerty goes here
		compression on
	}

The opening { must be on the same line. The closing } must be on it's own line.  
Lines starting with # are ignored.

Keys are:

### task
Followed by a required name. The top level element.

### archive
*required.* Path to where archive will be stored

### max-storage-time
The temporal depth of archive. For how long the versions will be stored. 
The value of the key is number with suffix. 6m means 'store for 6 month'. In this case versions older
than 6 month will be deleted. 
Suffixes are:  
	d - days  
	w - weeks  
	m - months  
	y - years  

### root
Where to start archiving from. The paths, stored in archive, will be relative to the `root`. Either `root` or `include` should be present

### include
What to add to archive. It is relative to `root` if root is present. Either `root` or `include` must be present

### exclude
What not to add to archive. It is relative to `root` if root is present.

### password
Password to archive

### compression 
Whether to compress the archive. Can be 'on' or 'off'. By default archives are not compressed.

### acl
Whether to store [ACLs][1] in archive. Can be 'on' or 'off'. By default ACLs are ignored.
[1]: https://en.wikipedia.org/wiki/Access-control_list




