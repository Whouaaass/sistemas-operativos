# ListPart
List partitions from a MBR/GPT disk

## Usage
List all the partitions of a disk:
```
listpart DEVICE_1 [DEVICE_2 ...]
```
```
listpart /dev/sda
```
you might replace /dev/sda with the disk name.
on linux you can use the `lsblk` command to get the disk name.

**This command needs root privileges to be run**.

List all the partitions of multiple disks:
```
listpart /dev/sda /dev/sdb
```
