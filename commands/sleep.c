/* sleep.c - Command to wait a specified number of seconds.  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2006,2007,2008  Free Software Foundation, Inc.
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

#include <grub/dl.h>
#include <grub/term.h>
#include <grub/time.h>
#include <grub/types.h>
#include <grub/misc.h>
#include <grub/normal.h>
#include <grub/machine/time.h>

static const struct grub_arg_option options[] =
  {
    {"verbose", 'v', 0, "verbose countdown", 0, 0},
    {"interruptible", 'i', 0, "interruptible with ESC", 0, 0},
    {0, 0, 0, 0, 0, 0}
  };

static grub_uint8_t x, y;

static void
do_print (int n)
{
  grub_gotoxy (x, y);
  /* NOTE: Do not remove the trailing space characters.
     They are required to clear the line.  */
  grub_printf ("%d    ", n);
}

/* Based on grub_millisleep() from kern/generic/millisleep.c.  */
static int
grub_interruptible_millisleep (grub_uint32_t ms)
{
  grub_uint64_t start;

  start = grub_get_time_ms ();

  while (grub_get_time_ms () - start < ms)
    if (grub_checkkey () >= 0 &&
	GRUB_TERM_ASCII_CHAR (grub_getkey ()) == GRUB_TERM_ESC)
      return 1;

  return 0;
}

static grub_err_t
grub_cmd_sleep (struct grub_arg_list *state, int argc, char **args)
{
  grub_uint16_t xy;
  int n;

  if (argc != 1)
    return grub_error (GRUB_ERR_BAD_ARGUMENT, "missing operand");

  n = grub_strtoul (args[0], 0, 10);

  if (n == 0)
    {
      /* Either `0' or broken input.  */
      return 0;
    }

  xy = grub_getxy ();
  x = xy >> 8;
  y = xy & 0xff;

  for (; n; n--)
    {
      if (state[0].set)
	do_print (n);

      if (state[1].set)
	{
	  if (grub_interruptible_millisleep (1000))
	    return 1;
	}
      else
	grub_millisleep (1000);
    }
  if (state[0].set)
    do_print (0);

  return 0;
}


GRUB_MOD_INIT(sleep)
{
  grub_register_command ("sleep", grub_cmd_sleep, GRUB_COMMAND_FLAG_BOTH,
			 "sleep NUMBER_OF_SECONDS", "Wait for a specified number of seconds", options);
}

GRUB_MOD_FINI(sleep)
{
  grub_unregister_command ("sleep");
}
