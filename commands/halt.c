/* halt.c - command to halt the computer.  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2005,2007,2008  Free Software Foundation, Inc.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <grub/normal.h>
#include <grub/dl.h>
#include <grub/machine/machine.h>

#if defined(GRUB_MACHINE_IEEE1275)
#include <grub/machine/kernel.h>
#elif defined(GRUB_MACHINE_EFI)
#include <grub/efi/efi.h>
#else
/* Platforms shipping standalone halt, such as coreboot.  */
#include <grub/cpu/halt.h>
#endif

static grub_err_t
grub_cmd_halt (struct grub_arg_list *state __attribute__ ((unused)),
	       int argc __attribute__ ((unused)),
	       char **args __attribute__ ((unused)))
{
  grub_halt ();
  return 0;
}


GRUB_MOD_INIT(halt)
{
  (void)mod;			/* To stop warning. */
  grub_register_command ("halt", grub_cmd_halt, GRUB_COMMAND_FLAG_BOTH,
			 "halt", "halts the computer.  This command does not"
			 " work on all firmware.", 0);
}

GRUB_MOD_FINI(halt)
{
  grub_unregister_command ("halt");
}
