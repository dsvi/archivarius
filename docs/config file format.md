# Config file

## Location

Archivarius looks for the archivarius.conf file at the locations:

	~/.config
	/usr/local/etc
	/etc

In the mentioned order.

## Format

Example:
 
	task home backup {
		archive /media/me/backup/
		max-storage-time 6m
		root /home/me/
		exclude {
			.cache
			.local/share/Trash
			.steam
			Steam
			.local/share/Steam
		}
		password ma qwerty goes here
		compression on
	}

The opening **{** must be on the same line. The closing **}** must be on it's own line.  
Lines starting with # are ignored.

### Keywords are:

### task
Followed by a required name. The top level element. Can be multiple in a file.

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
What to add to archive. It is relative to `root` if root is present. Either `root` or `include` must be present.
Don't add trailing `/` to directories.

### exclude
What not to add to archive. It is relative to `root` if root is present.
Don't add trailing `/` to directories.

### password
Password to archive

### compression 
Whether to compress the archive. Can be 'on' or 'off'. By default archives are not compressed.

### acl
Whether to store [ACLs][1] in archive. Can be 'on' or 'off'. By default ACLs are ignored.

[1]: https://en.wikipedia.org/wiki/Access-control_list




