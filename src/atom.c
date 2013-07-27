/**
 * vimb - a webkit based vim like browser.
 *
 * Copyright (C) 2012-2013 Daniel Carl
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#include <gdk/gdkx.h>
#include "config.h"
#include "atom.h"
#include "main.h"
#include "command.h"

#ifdef FEATURE_IPC
enum {
    ATOM_COMMAND,
    ATOM_OUT,
    ATOM_LAST
};

extern VbCore vb;
static Atom atoms[ATOM_LAST];
static Display *display;

static char *atom_get_string(Atom atom);
static void atom_set_string(Atom atom, char *str);
static void atom_set_string_list(Atom atom, char **list, int count);
static GdkFilterReturn event_filter_cb(GdkXEvent *xevent, GdkEvent *event, gpointer data);


void atom_init(void)
{
    display = gdk_x11_get_default_xdisplay();

    atoms[ATOM_COMMAND] = XInternAtom(display, "_VIMB_COMMAND", false);
    atoms[ATOM_OUT]     = XInternAtom(display, "_VIMB_OUT", false);

    gdk_window_set_events(GTK_WIDGET(vb.gui.window)->window, GDK_ALL_EVENTS_MASK);
    gdk_window_add_filter(GTK_WIDGET(vb.gui.window)->window, event_filter_cb, NULL);
}

/**
 * Retrieves the string property for given atom.
 *
 * Result must be freed with XFree.
 */
static char *atom_get_string(Atom atom)
{
    XTextProperty prop;
    if (!XGetTextProperty(display, GDK_WINDOW_XID(GTK_WIDGET(vb.gui.window)->window), &prop, atom)) {
        return NULL;
    }

    return (char*)prop.value;
}

static void atom_set_string(Atom atom, char *str)
{
    atom_set_string_list(atom, &str, 1);
}

static void atom_set_string_list(Atom atom, char **list, int count)
{
    XTextProperty prop;

    Xutf8TextListToTextProperty(display, list, count, XUTF8StringStyle, &prop);
    XSetTextProperty(display, GDK_WINDOW_XID(GTK_WIDGET(vb.gui.window)->window), &prop, atom);
    XFree(prop.value);
}

/**
 * Grep the property change events for known atoms and don't bubble them up.
 */
static GdkFilterReturn event_filter_cb(GdkXEvent *xevent, GdkEvent *event, gpointer data)
{
    XEvent *xe = (XEvent *)xevent;
    XPropertyEvent *ev;
    char *command;

    if (xe->type == PropertyNotify) {
        ev = &xe->xproperty;
        if (ev->state == PropertyNewValue && ev->atom == atoms[ATOM_COMMAND]) {
            /* run the command */
            command = atom_get_string(ev->atom);
            command_run_string(command);
            XFree(command);

            /* notify about the inputbox contents */
            atom_set_string(atoms[ATOM_OUT], (char *)GET_TEXT());

            return GDK_FILTER_REMOVE;
        }
    }
    return GDK_FILTER_CONTINUE;
}
#endif
