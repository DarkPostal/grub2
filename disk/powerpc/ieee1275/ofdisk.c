/* ofdisk.c - Open Firmware disk access.  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2004  Free Software Foundation, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <grub/misc.h>
#include <grub/disk.h>
#include <grub/mm.h>
#include <grub/machine/ieee1275.h>

static int
grub_ofdisk_iterate (int (*hook) (const char *name))
{
  int dev_iterate (struct grub_ieee1275_devalias *alias)
    {
      if (! grub_strcmp (alias->type, "block"))
	hook (alias->name);
      return 0;
    }

  grub_devalias_iterate (dev_iterate);
  return 0;
}

static grub_err_t
grub_ofdisk_open (const char *name, grub_disk_t disk)
{
  grub_ieee1275_phandle_t devalias;
  grub_ieee1275_phandle_t dev;
  grub_ieee1275_ihandle_t dev_ihandle = 0;
  char *devpath = 0;
  /* XXX: This should be large enough for any possible case.  */
  char prop[64];
  grub_size_t pathlen;
  int actual;

  if (grub_ieee1275_finddevice ("/aliases", &devalias))
    return grub_error (GRUB_ERR_UNKNOWN_DEVICE, "Can't read the aliases");
  
  grub_ieee1275_get_property_length (devalias, name, &pathlen);
  devpath = grub_malloc (pathlen);
  if (! devpath)
    return grub_errno;

  if (grub_ieee1275_get_property (devalias, name, devpath, pathlen, &actual))
    return grub_error (GRUB_ERR_UNKNOWN_DEVICE, "No such device alias");

  /* To access the complete disk add `:0'.  */
  grub_strcat (devpath, ":0");
  grub_ieee1275_open (devpath, &dev_ihandle);
  if (! dev_ihandle)
    return grub_error (GRUB_ERR_UNKNOWN_DEVICE, "Can't open device");
  
  if (grub_ieee1275_finddevice (devpath, &dev))
    {
      grub_error (GRUB_ERR_UNKNOWN_DEVICE, "Can't read device properties");
      goto fail;
    }

  if (grub_ieee1275_get_property (dev, "device_type", prop, sizeof (prop),
				  &actual))
    {
      grub_error (GRUB_ERR_BAD_DEVICE, "Can't read the device type");
      goto fail;
    }

  if (grub_strcmp (prop, "block"))
    {
      grub_error (GRUB_ERR_BAD_DEVICE, "Not a block device");
      goto fail;
    }

  /* XXX: There is no property to read the number of blocks.  There
     should be a property `#blocks', but it is not there.  Perhaps it
     is possible to use seek for this.  */
  disk->total_sectors = 0xFFFFFFFFUL;

  /* XXX: Is it ok to use this?  Perhaps it is better to use the path
     or some property.  */
  disk->id = dev;

  /* XXX: Read this, somehow.  */
  disk->has_partitions = 1;
  disk->data = (void *) dev_ihandle;

 fail:
  if (grub_errno)
    grub_ieee1275_close (dev_ihandle);
  grub_free (devpath);
  return grub_errno;
}

static void
grub_ofdisk_close (grub_disk_t disk)
{
  grub_ieee1275_close ((grub_ieee1275_ihandle_t) disk->data);
}

static grub_err_t
grub_ofdisk_read (grub_disk_t disk, unsigned long sector,
		  unsigned long size, char *buf)
{
  int status;
  int actual;
  unsigned long long pos;

  pos = (unsigned long long) sector * 512UL;

  grub_ieee1275_seek ((grub_ieee1275_ihandle_t) disk->data, (int) (pos >> 32),
		      (int) pos & 0xFFFFFFFFUL, &status);
  if (status != 0)
    return grub_error (GRUB_ERR_READ_ERROR,
		       "Seek error, can't seek block %d", sector);
  grub_ieee1275_read ((grub_ieee1275_ihandle_t) disk->data, buf,
		      size * 512UL, &actual);
  if (actual != actual)
    return grub_error (GRUB_ERR_READ_ERROR, "Read error on block: %d", sector);
    
  return 0;
}

static grub_err_t
grub_ofdisk_write (grub_disk_t disk __attribute ((unused)),
		   unsigned long sector __attribute ((unused)),
		   unsigned long size __attribute ((unused)),
		   const char *buf __attribute ((unused)))
{
  return GRUB_ERR_NOT_IMPLEMENTED_YET;
}

static struct grub_disk_dev grub_ofdisk_dev =
  {
    .name = "ofdisk",
    .iterate = grub_ofdisk_iterate,
    .open = grub_ofdisk_open,
    .close = grub_ofdisk_close,
    .read = grub_ofdisk_read,
    .write = grub_ofdisk_write,
    .next = 0
  };

void
grub_ofdisk_init (void)
{
  grub_disk_dev_register (&grub_ofdisk_dev);
}