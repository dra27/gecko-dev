/* cairo - a vector graphics library with display and print output
 *
 * Copyright © 2002 University of Southern California
 * Copyright © 2005 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 * The Original Code is the cairo graphics library.
 *
 * The Initial Developer of the Original Code is University of Southern
 * California.
 *
 * Contributor(s):
 *	Carl D. Worth <cworth@cworth.org>
 */

#include "cairoint.h"
#include "cairo-private.h"

#include "cairo-arc-private.h"
#include "cairo-path-data-private.h"

#define CAIRO_TOLERANCE_MINIMUM	0.0002 /* We're limited by 16 bits of sub-pixel precision */

#include <assert.h>
#ifdef NDEBUG
#define CAIRO_CHECK_SANITY(cr) 
#else
static int 
cairo_sane_state (cairo_t *cr)
{    
    if (cr == NULL)
	return 0;

    switch (cr->status) {
    case CAIRO_STATUS_SUCCESS:
    case CAIRO_STATUS_NO_MEMORY:
    case CAIRO_STATUS_INVALID_RESTORE:
    case CAIRO_STATUS_INVALID_POP_GROUP:
    case CAIRO_STATUS_NO_CURRENT_POINT:
    case CAIRO_STATUS_INVALID_MATRIX:
    case CAIRO_STATUS_NO_TARGET_SURFACE:
    case CAIRO_STATUS_NULL_POINTER:
    case CAIRO_STATUS_INVALID_STRING:
    case CAIRO_STATUS_INVALID_PATH_DATA:
	break;
    default:
	return 0;
    }
    return 1;
}
#define CAIRO_CHECK_SANITY(cr) assert(cairo_sane_state ((cr)))
#endif

/*
 * cairo_create:
 * @target: target surface for the context
 * 
 * Creates a new #cairo_t with all graphics state parameters set to
 * default values and with @target as a target surface. The target
 * surface should be constructed with a backend-specific function such
 * as cairo_image_surface_create (or any other
 * cairo_<backend>_surface_create variant).
 *
 * This function references @target, so you can immediately
 * call cairo_surface_destroy() on it if you don't need to
 * maintain a separate reference to it.
 *
 * Note that there are restrictions on using the same surface in
 * multiple contexts at the same time. If, after creating @cr_a with
 * @surface you also create @cr_b with the same surface, you must
 * ensure that @cr_b has finished using @surface before resuming use
 * of @cr_a. Currently, the only way time at which this is guaranteed
 * is when the the last reference to @cr_b is released with
 * cairo_destroy(). (XXX: We need to add a cairo_finish() call to
 * provide a way to achieve this explicitly). See also the
 * %CAIRO_STATUS_BAD_NESTING status.
 * 
 * Return value: a newly allocated #cairo_t with a reference
 *  count of 1. The initial reference count should be released
 *  with cairo_destroy() when you are done using the #cairo_t.
 */
cairo_t *
cairo_create (cairo_surface_t *target)
{
    cairo_t *cr;

    cr = malloc (sizeof (cairo_t));
    if (cr == NULL)
	return NULL;

    cr->status = CAIRO_STATUS_SUCCESS;
    cr->ref_count = 1;

    _cairo_path_fixed_init (&cr->path);

    cr->gstate = _cairo_gstate_create (target);
    if (cr->gstate == NULL)
	cr->status = CAIRO_STATUS_NO_MEMORY;

    CAIRO_CHECK_SANITY (cr);
    return cr;
}

/**
 * cairo_reference:
 * @cr: a #cairo_t
 * 
 * Increases the reference count on @cr by one. This prevents
 * @cr from being destroyed until a matching call to cairo_destroy() 
 * is made.
 **/
void
cairo_reference (cairo_t *cr)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->ref_count++;
    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_destroy:
 * @cr: a #cairo_t
 * 
 * Decreases the reference count on @cr by one. If the result
 * is zero, then @cr and all associated resources are freed.
 * See cairo_reference().
 **/
void
cairo_destroy (cairo_t *cr)
{
    CAIRO_CHECK_SANITY (cr);
    cr->ref_count--;
    if (cr->ref_count)
	return;

    while (cr->gstate) {
	cairo_gstate_t *tmp = cr->gstate;
	cr->gstate = tmp->next;

	_cairo_gstate_destroy (tmp);
    }

    _cairo_path_fixed_fini (&cr->path);

    free (cr);
}

/**
 * cairo_save:
 * @cr: a #cairo_t
 * 
 * Makes a copy of the current state of @cr and saves it
 * on an internal stack of saved states for @cr. When
 * cairo_restore() is called, @cr will be restored to
 * the saved state. Multiple calls to cairo_save() and
 * cairo_restore() can be nested; each call to cairo_restore()
 * restores the state from the matching paired cairo_save().
 *
 * It isn't necessary to clear all saved states before
 * a #cairo_t is freed. If the reference count of a #cairo_t
 * drops to zero in response to a call to cairo_destroy(),
 * any saved states will be freed along with the #cairo_t.
 **/
void
cairo_save (cairo_t *cr)
{
    cairo_gstate_t *top;

    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    top = _cairo_gstate_clone (cr->gstate);

    if (top == NULL) {
	cr->status = CAIRO_STATUS_NO_MEMORY;
	CAIRO_CHECK_SANITY (cr);
	return;
    }

    top->next = cr->gstate;
    cr->gstate = top;
    CAIRO_CHECK_SANITY (cr);
}
slim_hidden_def(cairo_save);

/**
 * cairo_restore:
 * @cr: a #cairo_t
 * 
 * Restores @cr to the state saved by a preceding call to
 * cairo_save() and removes that state from the stack of
 * saved states.
 **/
void
cairo_restore (cairo_t *cr)
{
    cairo_gstate_t *top;

    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    top = cr->gstate;
    cr->gstate = top->next;

    _cairo_gstate_destroy (top);

    if (cr->gstate == NULL)
	cr->status = CAIRO_STATUS_INVALID_RESTORE;
    
    if (cr->status)
	return;
   
    CAIRO_CHECK_SANITY (cr);
}
slim_hidden_def(cairo_restore);

/* XXX: I want to rethink this API
void
cairo_push_group (cairo_t *cr)
{
    if (cr->status)
	return;

    cr->status = cairoPush (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_begin_group (cr->gstate);
}

void
cairo_pop_group (cairo_t *cr)
{
    if (cr->status)
	return;

    cr->status = _cairo_gstate_end_group (cr->gstate);
    if (cr->status)
	return;

    cr->status = cairoPop (cr);
}
*/

/**
 * cairo_set_operator:
 * @cr: a #cairo_t
 * @op: a compositing operator, specified as a #cairo_operator_t
 * 
 * Sets the compositing operator to be used for all drawing
 * operations. See #cairo_operator_t for details on the semantics of
 * each available drawing operator.
 *
 * XXX: I'd also like to direct the reader's attention to some
 * (not-yet-written) section on cairo's imaging model. How would I do
 * that if such a section existed? (cworth).
 **/
void
cairo_set_operator (cairo_t *cr, cairo_operator_t op)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_set_operator (cr->gstate, op);
    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_set_source_rgb
 * @cr: a cairo context
 * @red: red component of color
 * @green: green component of color
 * @blue: blue component of color
 * 
 * Sets the source pattern within @cr to an opaque color. This opaque
 * color will then be used for any subsequent drawing operation until
 * a new source pattern is set.
 *
 * The color components are floating point numbers in the range 0 to
 * 1. If the values passed in are outside that range, they will be
 * clamped.
 **/
void
cairo_set_source_rgb (cairo_t *cr, double red, double green, double blue)
{
    cairo_color_t color;

    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    _cairo_restrict_value (&red, 0.0, 1.0);
    _cairo_restrict_value (&green, 0.0, 1.0);
    _cairo_restrict_value (&blue, 0.0, 1.0);

    _cairo_color_init_rgb (&color, red, green, blue);

    cr->status = _cairo_gstate_set_source_solid (cr->gstate, &color);
    
    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_set_source_rgba:
 * @cr: a cairo context
 * @red: red component of color
 * @green: green component of color
 * @blue: blue component of color
 * @alpha: alpha component of color
 * 
 * Sets the source pattern within @cr to a translucent color. This
 * color will then be used for any subsequent drawing operation until
 * a new source pattern is set.
 *
 * The color and alpha components are floating point numbers in the
 * range 0 to 1. If the values passed in are outside that range, they
 * will be clamped.
 **/
void
cairo_set_source_rgba (cairo_t *cr,
		       double red, double green, double blue,
		       double alpha)
{
    cairo_color_t color;

    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    _cairo_restrict_value (&red,   0.0, 1.0);
    _cairo_restrict_value (&green, 0.0, 1.0);
    _cairo_restrict_value (&blue,  0.0, 1.0);
    _cairo_restrict_value (&alpha, 0.0, 1.0);

    _cairo_color_init_rgba (&color, red, green, blue, alpha);

    cr->status = _cairo_gstate_set_source_solid (cr->gstate, &color);
    
    CAIRO_CHECK_SANITY (cr);
}

void
cairo_set_source_surface (cairo_t	  *cr,
			  cairo_surface_t *surface,
			  double	   x,
			  double	   y)
{
    cairo_pattern_t *pattern;
    cairo_matrix_t matrix;

    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    pattern = cairo_pattern_create_for_surface (surface);
    if (!pattern) {
	cr->status = CAIRO_STATUS_NO_MEMORY;
	return;
    }

    cairo_matrix_init_translate (&matrix, -x, -y);
    cairo_pattern_set_matrix (pattern, &matrix);

    cairo_set_source (cr, pattern);
    cairo_pattern_destroy (pattern);

    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_set_source
 * @cr: a cairo context
 * @source: a #cairo_pattern_t to be used as the source for
 * subsequent drawing operations.
 * 
 * Sets the source pattern within @cr to @source. This pattern
 * will then be used for any subsequent drawing operation until a new
 * source pattern is set.
 *
 * XXX: I'd also like to direct the reader's attention to some
 * (not-yet-written) section on cairo's imaging model. How would I do
 * that if such a section existed? (cworth).
 **/
void
cairo_set_source (cairo_t *cr, cairo_pattern_t *source)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_set_source (cr->gstate, source);
    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_get_source:
 * @cr: a cairo context
 * 
 * Gets the current source pattern for @cr.
 * 
 * Return value: the current source pattern. This object is owned by
 * cairo. To keep a reference to it, you must call
 * cairo_pattern_reference().
 **/
cairo_pattern_t *
cairo_get_source (cairo_t *cr)
{
    CAIRO_CHECK_SANITY (cr);
    /* XXX: We'll want to do something like this:
    if (cr->status)
	return cairo_pattern_nil;
    */

    return _cairo_gstate_get_source (cr->gstate);
}

/**
 * cairo_set_tolerance:
 * @cr: a #cairo_t
 * @tolerance: the tolerance, in device units (typically pixels)
 * 
 * Sets the tolerance used when converting paths into trapezoids.
 * Curved segments of the path will be subdivided until the maximum
 * deviation between the original path and the polygonal approximation
 * is less than @tolerance. The default value is 0.1. A larger
 * value will give better performance, a smaller value, better
 * appearance. (Reducing the value from the default value of 0.1
 * is unlikely to improve appearance significantly.)
 **/
void
cairo_set_tolerance (cairo_t *cr, double tolerance)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    _cairo_restrict_value (&tolerance, CAIRO_TOLERANCE_MINIMUM, tolerance);

    cr->status = _cairo_gstate_set_tolerance (cr->gstate, tolerance);
    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_set_fill_rule:
 * @cr: a #cairo_t
 * @fill_rule: a fill rule, specified as a #cairo_fill_rule_t
 * 
 * Set the current fill rule within the cairo context. The fill rule
 * is used to determine which regions are inside or outside a complex
 * (potentially self-intersecting) path. The current fill rule affects
 * both cairo_fill and cairo_clip. See #cairo_fill_rule_t for details
 * on the semantics of each available fill rule.
 **/
void
cairo_set_fill_rule (cairo_t *cr, cairo_fill_rule_t fill_rule)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_set_fill_rule (cr->gstate, fill_rule);
    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_set_line_width:
 * @cr: a #cairo_t
 * @width: a line width, as a user-space value
 * 
 * Sets the current line width within the cairo context. The line
 * width specifies the diameter of a pen that is circular in
 * user-space.
 *
 * As with the other stroke parameters, the current line cap style is
 * examined by cairo_stroke(), cairo_stroke_extents(), and
 * cairo_stroke_to_path(), but does not have any effect during path
 * construction.
 **/
void
cairo_set_line_width (cairo_t *cr, double width)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    _cairo_restrict_value (&width, 0.0, width);

    cr->status = _cairo_gstate_set_line_width (cr->gstate, width);
    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_set_line_cap:
 * @cr: a cairo context, as a #cairo_t
 * @line_cap: a line cap style, as a #cairo_line_cap_t
 * 
 * Sets the current line cap style within the cairo context. See
 * #cairo_line_cap_t for details about how the available line cap
 * styles are drawn.
 *
 * As with the other stroke parameters, the current line cap style is
 * examined by cairo_stroke(), cairo_stroke_extents(), and
 * cairo_stroke_to_path(), but does not have any effect during path
 * construction.
 **/
void
cairo_set_line_cap (cairo_t *cr, cairo_line_cap_t line_cap)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_set_line_cap (cr->gstate, line_cap);
    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_set_line_join:
 * @cr: a cairo context, as a #cairo_t
 * @line_join: a line joint style, as a #cairo_line_join_t
 *
 * Sets the current line join style within the cairo context. See
 * #cairo_line_join_t for details about how the available line join
 * styles are drawn.
 *
 * As with the other stroke parameters, the current line join style is
 * examined by cairo_stroke(), cairo_stroke_extents(), and
 * cairo_stroke_to_path(), but does not have any effect during path
 * construction.
 **/
void
cairo_set_line_join (cairo_t *cr, cairo_line_join_t line_join)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_set_line_join (cr->gstate, line_join);
    CAIRO_CHECK_SANITY (cr);
}

void
cairo_set_dash (cairo_t *cr, double *dashes, int ndash, double offset)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_set_dash (cr->gstate, dashes, ndash, offset);
    CAIRO_CHECK_SANITY (cr);
}

void
cairo_set_miter_limit (cairo_t *cr, double limit)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_set_miter_limit (cr->gstate, limit);
    CAIRO_CHECK_SANITY (cr);
}


/**
 * cairo_translate:
 * @cr: a cairo context
 * @tx: amount to translate in the X direction
 * @ty: amount to translate in the Y direction
 * 
 * Modifies the current transformation matrix (CTM) by tanslating the
 * user-space origin by (@tx, @ty). This offset is interpreted as a
 * user-space coordinate according to the CTM in place before the new
 * call to cairo_translate. In other words, the translation of the
 * user-space origin takes place after any existing transformation.
 **/
void
cairo_translate (cairo_t *cr, double tx, double ty)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_translate (cr->gstate, tx, ty);
    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_scale:
 * @cr: a cairo context
 * @sx: scale factor for the X dimension
 * @sy: scale factor for the Y dimension
 * 
 * Modifies the current transformation matrix (CTM) by scaling the X
 * and Y user-space axes by @sx and @sy respectively. The scaling of
 * the axes takes place after any existing transformation of user
 * space.
 **/
void
cairo_scale (cairo_t *cr, double sx, double sy)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_scale (cr->gstate, sx, sy);
    CAIRO_CHECK_SANITY (cr);
}


/**
 * cairo_rotate:
 * @cr: a cairo context
 * @angle: angle (in radians) by which the user-space axes will be
 * rotated
 * 
 * Modifies the current transformation matrix (CTM) by rotating the
 * user-space axes by @angle radians. The rotation of the axes takes
 * places after any existing transformation of user space. The
 * rotation direction for positive angles is from the positive X axis
 * toward the positive Y axis.
 **/
void
cairo_rotate (cairo_t *cr, double angle)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_rotate (cr->gstate, angle);
    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_transform:
 * @cr: a cairo context
 * @matrix: a transformation to be applied to the user-space axes
 * 
 * Modifies the current transformation matrix (CTM) by applying
 * @matrix as an additional transformation. The new transformation of
 * user space takes place after any existing transformation.
 **/
void
cairo_transform (cairo_t	      *cr,
		 const cairo_matrix_t *matrix)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_transform (cr->gstate, matrix);
    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_set_matrix:
 * @cr: a cairo context
 * @matrix: a transformation matrix from user space to device space
 * 
 * Modifies the current transformation matrix (CTM) by setting it
 * equal to @matrix.
 **/
void
cairo_set_matrix (cairo_t	       *cr,
		  const cairo_matrix_t *matrix)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_set_matrix (cr->gstate, matrix);
    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_identity_matrix:
 * @cr: a cairo context
 * 
 * Resets the current transformation matrix (CTM) by setting it equal
 * to the identity matrix. That is, the user-space and device-space
 * axes will be aligned and one user-space unit will transform to one
 * device-space unit.
 **/
void
cairo_identity_matrix (cairo_t *cr)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_identity_matrix (cr->gstate);
    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_user_to_device:
 * @cr: a cairo context
 * @x: X value of coordinate (in/out parameter)
 * @y: Y value of coordinate (in/out parameter)
 * 
 * Transform a coordinate from user space to device space by
 * multiplying the given point by the current transformation matrix
 * (CTM).
 **/
void
cairo_user_to_device (cairo_t *cr, double *x, double *y)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_user_to_device (cr->gstate, x, y);
    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_user_to_device_distance:
 * @cr: a cairo context
 * @dx: X component of a distance vector (in/out parameter)
 * @dy: Y component of a distance vector (in/out parameter)
 * 
 * Transform a distance vector from user space to device space. This
 * function is similar to cairo_user_to_device() except that the
 * translation components of the CTM will be ignored when transforming
 * (@dx,@dy).
 **/
void
cairo_user_to_device_distance (cairo_t *cr, double *dx, double *dy)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_user_to_device_distance (cr->gstate, dx, dy);
    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_device_to_user:
 * @cr: a cairo
 * @x: X value of coordinate (in/out parameter)
 * @y: Y value of coordinate (in/out parameter)
 * 
 * Transform a coordinate from device space to user space by
 * multiplying the given point by the inverse of the current
 * transformation matrix (CTM).
 **/
void
cairo_device_to_user (cairo_t *cr, double *x, double *y)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_device_to_user (cr->gstate, x, y);
    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_device_to_user_distance:
 * @cr: a cairo context
 * @dx: X component of a distance vector (in/out parameter)
 * @dy: Y component of a distance vector (in/out parameter)
 * 
 * Transform a distance vector from device space to user space. This
 * function is similar to cairo_device_to_user() except that the
 * translation components of the inverse CTM will be ignored when
 * transforming (@dx,@dy).
 **/
void
cairo_device_to_user_distance (cairo_t *cr, double *dx, double *dy)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_device_to_user_distance (cr->gstate, dx, dy);
    CAIRO_CHECK_SANITY (cr);
}

void
cairo_new_path (cairo_t *cr)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    _cairo_path_fixed_fini (&cr->path);

    CAIRO_CHECK_SANITY (cr);
}
slim_hidden_def(cairo_new_path);

void
cairo_move_to (cairo_t *cr, double x, double y)
{
    cairo_fixed_t x_fixed, y_fixed;

    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    _cairo_gstate_user_to_backend (cr->gstate, &x, &y);
    x_fixed = _cairo_fixed_from_double (x);
    y_fixed = _cairo_fixed_from_double (y);

    cr->status = _cairo_path_fixed_move_to (&cr->path, x_fixed, y_fixed);
    
    CAIRO_CHECK_SANITY (cr);
}
slim_hidden_def(cairo_move_to);

void
cairo_line_to (cairo_t *cr, double x, double y)
{
    cairo_fixed_t x_fixed, y_fixed;

    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    _cairo_gstate_user_to_backend (cr->gstate, &x, &y);
    x_fixed = _cairo_fixed_from_double (x);
    y_fixed = _cairo_fixed_from_double (y);

    cr->status = _cairo_path_fixed_line_to (&cr->path, x_fixed, y_fixed);

    CAIRO_CHECK_SANITY (cr);
}

void
cairo_curve_to (cairo_t *cr,
		double x1, double y1,
		double x2, double y2,
		double x3, double y3)
{
    cairo_fixed_t x1_fixed, y1_fixed;
    cairo_fixed_t x2_fixed, y2_fixed;
    cairo_fixed_t x3_fixed, y3_fixed;
	
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    _cairo_gstate_user_to_backend (cr->gstate, &x1, &y1);
    _cairo_gstate_user_to_backend (cr->gstate, &x2, &y2);
    _cairo_gstate_user_to_backend (cr->gstate, &x3, &y3);

    x1_fixed = _cairo_fixed_from_double (x1);
    y1_fixed = _cairo_fixed_from_double (y1);

    x2_fixed = _cairo_fixed_from_double (x2);
    y2_fixed = _cairo_fixed_from_double (y2);

    x3_fixed = _cairo_fixed_from_double (x3);
    y3_fixed = _cairo_fixed_from_double (y3);

    cr->status = _cairo_path_fixed_curve_to (&cr->path,
					     x1_fixed, y1_fixed,
					     x2_fixed, y2_fixed,
					     x3_fixed, y3_fixed);

    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_arc:
 * @cr: a cairo context
 * @xc: X position of the center of the arc
 * @yc: Y position of the center of the arc
 * @radius: the radius of the arc
 * @angle1: the start angle, in radians
 * @angle2: the end angle, in radians
 * 
 * Adds an arc from @angle1 to @angle2 to the current path. If there
 * is a current point, that point is connected to the start of the arc
 * by a straight line segment. Angles are measured in radians with an
 * angle of 0 along the X axis and an angle of %M_PI radians (90
 * degrees) along the Y axis, so with the default transformation
 * matrix, positive angles are clockwise. (To convert from degrees to
 * radians, use <literal>degrees * (M_PI / 180.)</literal>.)  This
 * function gives the arc in the direction of increasing angle; see
 * cairo_arc_negative() to get the arc in the direction of decreasing
 * angle.
 *
 * A full arc is drawn as a circle. To make an oval arc, you can scale
 * the current transformation matrix by different amounts in the X and
 * Y directions. For example, to draw a full oval in the box given
 * by @x, @y, @width, @height:
 
 * <informalexample><programlisting>
 * cairo_save (cr);
 * cairo_translate (x + width / 2., y + height / 2.);
 * cairo_scale (1. / (height / 2.), 1. / (width / 2.));
 * cairo_arc (cr, 0., 0., 1., 0., 2 * M_PI);
 * cairo_restore (cr);
 * </programlisting></informalexample>
 **/
void
cairo_arc (cairo_t *cr,
	   double xc, double yc,
	   double radius,
	   double angle1, double angle2)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    /* Do nothing, successfully, if radius is <= 0 */
    if (radius <= 0.0)
	return;

    while (angle2 < angle1)
	angle2 += 2 * M_PI;

    cairo_line_to (cr,
		   xc + radius * cos (angle1),
		   yc + radius * sin (angle1));

    _cairo_arc_path (cr, xc, yc, radius,
		     angle1, angle2);

    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_arc_negative:
 * @cr: a cairo context
 * @xc: X position of the center of the arc
 * @yc: Y position of the center of the arc
 * @radius: the radius of the arc
 * @angle1: the start angle, in radians
 * @angle2: the end angle, in radians
 * 
 * Adds an arc from @angle1 to @angle2 to the current path. The
 * function behaves identically to cairo_arc() except that instead of
 * giving the arc in the direction of increasing angle, it gives
 * the arc in the direction of decreasing angle.
 **/
void
cairo_arc_negative (cairo_t *cr,
		    double xc, double yc,
		    double radius,
		    double angle1, double angle2)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    /* Do nothing, successfully, if radius is <= 0 */
    if (radius <= 0.0)
	return;

    while (angle2 > angle1)
	angle2 -= 2 * M_PI;

    cairo_line_to (cr,
		   xc + radius * cos (angle1),
		   yc + radius * sin (angle1));

     _cairo_arc_path_negative (cr, xc, yc, radius,
			       angle1, angle2);

    CAIRO_CHECK_SANITY (cr);
}

/* XXX: NYI
void
cairo_arc_to (cairo_t *cr,
	      double x1, double y1,
	      double x2, double y2,
	      double radius)
{
    if (cr->status)
	return;

    cr->status = _cairo_gstate_arc_to (cr->gstate,
				       x1, y1,
				       x2, y2,
				       radius);
}
*/

void
cairo_rel_move_to (cairo_t *cr, double dx, double dy)
{
    cairo_fixed_t dx_fixed, dy_fixed;

    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    _cairo_gstate_user_to_device_distance (cr->gstate, &dx, &dy);
    dx_fixed = _cairo_fixed_from_double (dx);
    dy_fixed = _cairo_fixed_from_double (dy);

    cr->status = _cairo_path_fixed_rel_move_to (&cr->path, dx_fixed, dy_fixed);

    CAIRO_CHECK_SANITY (cr);
}

void
cairo_rel_line_to (cairo_t *cr, double dx, double dy)
{
    cairo_fixed_t dx_fixed, dy_fixed;

    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    _cairo_gstate_user_to_device_distance (cr->gstate, &dx, &dy);
    dx_fixed = _cairo_fixed_from_double (dx);
    dy_fixed = _cairo_fixed_from_double (dy);

    cr->status = _cairo_path_fixed_rel_line_to (&cr->path, dx_fixed, dy_fixed);

    CAIRO_CHECK_SANITY (cr);
}
slim_hidden_def(cairo_rel_line_to);

void
cairo_rel_curve_to (cairo_t *cr,
		    double dx1, double dy1,
		    double dx2, double dy2,
		    double dx3, double dy3)
{
    cairo_fixed_t dx1_fixed, dy1_fixed;
    cairo_fixed_t dx2_fixed, dy2_fixed;
    cairo_fixed_t dx3_fixed, dy3_fixed;

    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    _cairo_gstate_user_to_device_distance (cr->gstate, &dx1, &dy1);
    _cairo_gstate_user_to_device_distance (cr->gstate, &dx2, &dy2);
    _cairo_gstate_user_to_device_distance (cr->gstate, &dx3, &dy3);

    dx1_fixed = _cairo_fixed_from_double (dx1);
    dy1_fixed = _cairo_fixed_from_double (dy1);

    dx2_fixed = _cairo_fixed_from_double (dx2);
    dy2_fixed = _cairo_fixed_from_double (dy2);

    dx3_fixed = _cairo_fixed_from_double (dx3);
    dy3_fixed = _cairo_fixed_from_double (dy3);

    cr->status = _cairo_path_fixed_rel_curve_to (&cr->path,
						 dx1_fixed, dy1_fixed,
						 dx2_fixed, dy2_fixed,
						 dx3_fixed, dy3_fixed);

    CAIRO_CHECK_SANITY (cr);
}

void
cairo_rectangle (cairo_t *cr,
		 double x, double y,
		 double width, double height)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cairo_move_to (cr, x, y);
    cairo_rel_line_to (cr, width, 0);
    cairo_rel_line_to (cr, 0, height);
    cairo_rel_line_to (cr, -width, 0);
    cairo_close_path (cr);
    CAIRO_CHECK_SANITY (cr);
}

/* XXX: NYI
void
cairo_stroke_to_path (cairo_t *cr)
{
    if (cr->status)
	return;

    cr->status = _cairo_gstate_stroke_path (cr->gstate);
}
*/

void
cairo_close_path (cairo_t *cr)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_path_fixed_close_path (&cr->path);

    CAIRO_CHECK_SANITY (cr);
}
slim_hidden_def(cairo_close_path);

/**
 * cairo_paint:
 * @cr: a cairo context
 * 
 * A drawing operator that paints the current source everywhere within
 * the current clip region.
 **/
void
cairo_paint (cairo_t *cr)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_paint (cr->gstate);

    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_paint_with_alpha:
 * @cr: a cairo context
 * @alpha: alpha value, between 0 (transparent) and 1 (opaque)
 * 
 * A drawing operator that paints the current source everywhere within
 * the current clip region using a mask of constant alpha value
 * @alpha. The effect is similar to cairo_paint(), but the drawing
 * is faded out using the alpha value.
 **/
void
cairo_paint_with_alpha (cairo_t *cr,
			double   alpha)
{
    cairo_color_t color;
    cairo_pattern_union_t pattern;
  
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    if (alpha == 1.0) {
        cr->status = _cairo_gstate_paint (cr->gstate);
    } else {
        _cairo_color_init_rgba (&color, 1., 1., 1., alpha);
        _cairo_pattern_init_solid (&pattern.solid, &color);

        cr->status = _cairo_gstate_mask (cr->gstate, &pattern.base);

        _cairo_pattern_fini (&pattern.base);
    }

    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_mask:
 * @cr: a cairo context
 * @pattern: a #cairo_pattern_t
 *
 * A drawing operator that paints the current source
 * using the alpha channel of @pattern as a mask. (Opaque
 * areas of @mask are painted with the source, transparent
 * areas are not painted.)
 */
void
cairo_mask (cairo_t         *cr,
	    cairo_pattern_t *pattern)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_mask (cr->gstate, pattern);

    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_mask_surface:
 * @cr: a cairo context
 * @surface: a #cairo_surface_t
 * @surface_x: X coordinate at which to place the origin of @surface
 * @surface_y: Y coordinate at which to place the origin of @surface
 *
 * A drawing operator that paints the current source
 * using the alpha channel of @surface as a mask. (Opaque
 * areas of @surface are painted with the source, transparent
 * areas are not painted.) 
 */
void
cairo_mask_surface (cairo_t         *cr,
		    cairo_surface_t *surface,
		    double           surface_x,
		    double           surface_y)
{
    cairo_pattern_t *pattern;
    cairo_matrix_t matrix;

    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    pattern = cairo_pattern_create_for_surface (surface);
    if (!pattern) {
	cr->status = CAIRO_STATUS_NO_MEMORY;
	return;
    }

    cairo_matrix_init_translate (&matrix, - surface_x, - surface_y);
    cairo_pattern_set_matrix (pattern, &matrix);

    cairo_mask (cr, pattern);
    
    cairo_pattern_destroy (pattern);
}

/**
 * cairo_stroke:
 * @cr: a cairo context
 * 
 * A drawing operator that strokes the current path according to the
 * current line width, line join, line cap, and dash settings. After
 * cairo_stroke, the current path will be cleared from the cairo
 * context. See cairo_set_line_width(), cairo_set_line_join(),
 * cairo_set_line_cap(), cairo_set_dash(), and
 * cairo_stroke_preserve().
 **/
void
cairo_stroke (cairo_t *cr)
{
    cairo_stroke_preserve (cr);

    cairo_new_path (cr);
}

/**
 * cairo_stroke_preserve:
 * @cr: a cairo context
 * 
 * A drawing operator that strokes the current path according to the
 * current line width, line join, line cap, and dash settings. Unlike
 * cairo_stroke(), cairo_stroke_preserve preserves the path within the
 * cairo context.
 *
 * See cairo_set_line_width(), cairo_set_line_join(),
 * cairo_set_line_cap(), cairo_set_dash(), and
 * cairo_stroke_preserve().
 **/
void
cairo_stroke_preserve (cairo_t *cr)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_stroke (cr->gstate, &cr->path);

    CAIRO_CHECK_SANITY (cr);
}
slim_hidden_def(cairo_stroke_preserve);

/**
 * cairo_fill:
 * @cr: a cairo context
 * 
 * A drawing operator that fills the current path according to the
 * current fill rule. After cairo_fill, the current path will be
 * cleared from the cairo context. See cairo_set_fill_rule() and
 * cairo_fill_preserve().
 **/
void
cairo_fill (cairo_t *cr)
{
    cairo_fill_preserve (cr);

    cairo_new_path (cr);
}

/**
 * cairo_fill_preserve:
 * @cr: a cairo context
 * 
 * A drawing operator that fills the current path according to the
 * current fill rule. Unlike cairo_fill(), cairo_fill_preserve
 * preserves the path within the cairo context.
 *
 * See cairo_set_fill_rule() and cairo_fill().
 **/
void
cairo_fill_preserve (cairo_t *cr)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_fill (cr->gstate, &cr->path);

    CAIRO_CHECK_SANITY (cr);
}
slim_hidden_def(cairo_fill_preserve);

void
cairo_copy_page (cairo_t *cr)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_copy_page (cr->gstate);
    CAIRO_CHECK_SANITY (cr);
}

void
cairo_show_page (cairo_t *cr)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_show_page (cr->gstate);
    CAIRO_CHECK_SANITY (cr);
}

cairo_bool_t
cairo_in_stroke (cairo_t *cr, double x, double y)
{
    int inside;

    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return 0;

    cr->status = _cairo_gstate_in_stroke (cr->gstate,
					  &cr->path,
					  x, y, &inside);

    CAIRO_CHECK_SANITY (cr);

    if (cr->status)
	return 0;

    return inside;
}

int
cairo_in_fill (cairo_t *cr, double x, double y)
{
    int inside;

    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return 0;

    cr->status = _cairo_gstate_in_fill (cr->gstate,
					&cr->path,
					x, y, &inside);

    CAIRO_CHECK_SANITY (cr);

    if (cr->status)
	return 0;

    return inside;
}

void
cairo_stroke_extents (cairo_t *cr,
                      double *x1, double *y1, double *x2, double *y2)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;
    
    cr->status = _cairo_gstate_stroke_extents (cr->gstate,
					       &cr->path,
					       x1, y1, x2, y2);
    CAIRO_CHECK_SANITY (cr);
}

void
cairo_fill_extents (cairo_t *cr,
                    double *x1, double *y1, double *x2, double *y2)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;
    
    cr->status = _cairo_gstate_fill_extents (cr->gstate,
					     &cr->path,
					     x1, y1, x2, y2);
    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_clip:
 * @cr: a cairo context
 * 
 * Establishes a new clip region by intersecting the current clip
 * region with the current path as it would be filled by cairo_fill()
 * and according to the current fill rule (see cairo_set_fill_rule()).
 *
 * After cairo_clip, the current path will be cleared from the cairo
 * context.
 *
 * The current clip region affects all drawing operations by
 * effectively masking out any changes to the surface that are outside
 * the current clip region.
 *
 * Calling cairo_clip() can only make the clip region smaller, never
 * larger. But the current clip is part of the graphics state, so a
 * tempoarary restriction of the clip region can be achieved by
 * calling cairo_clip() within a cairo_save()/cairo_restore()
 * pair. The only other means of increasing the size of the clip
 * region is cairo_reset_clip().
 **/
void
cairo_clip (cairo_t *cr)
{
    cairo_clip_preserve (cr);

    cairo_new_path (cr);
}

/**
 * cairo_clip_preserve:
 * @cr: a cairo context
 * 
 * Establishes a new clip region by intersecting the current clip
 * region with the current path as it would be filled by cairo_fill()
 * and according to the current fill rule (see cairo_set_fill_rule()).
 *
 * Unlike cairo_clip(), cairo_clip_preserve preserves the path within
 * the cairo context.
 *
 * The current clip region affects all drawing operations by
 * effectively masking out any changes to the surface that are outside
 * the current clip region.
 *
 * Calling cairo_clip() can only make the clip region smaller, never
 * larger. But the current clip is part of the graphics state, so a
 * tempoarary restriction of the clip region can be achieved by
 * calling cairo_clip() within a cairo_save()/cairo_restore()
 * pair. The only other means of increasing the size of the clip
 * region is cairo_reset_clip().
 **/
void
cairo_clip_preserve (cairo_t *cr)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_clip (cr->gstate, &cr->path);
    CAIRO_CHECK_SANITY (cr);
}
slim_hidden_def(cairo_clip_preserve);

/**
 * cairo_reset_clip:
 * @cr: a cairo context
 * 
 * Reset the current clip region to its original, unrestricted
 * state. That is, set the clip region to an infinitely large shape
 * containing the target surface. Equivalently, if infinity is too
 * hard to grasp, one can imagine the clip region being reset to the
 * exact bounds of the target surface.
 *
 * Note that code meant to be reusable should not call
 * cairo_reset_clip() as it will cause results unexpected by
 * higher-level code which calls cairo_clip(). Consider using
 * cairo_save() and cairo_restore() around cairo_clip() as a more
 * robust means of temporarily restricting the clip region.
 **/
void
cairo_reset_clip (cairo_t *cr)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_reset_clip (cr->gstate);
    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_select_font_face:
 * @cr: a #cairo_t
 * @family: a font family name, encoded in UTF-8
 * @slant: the slant for the font
 * @weight: the weight for the font
 * 
 * Selects a family and style of font from a simplified description as
 * a family name, slant and weight. This function is meant to be used
 * only for applications with simple font needs: Cairo doesn't provide
 * for operations such as listing all available fonts on the system,
 * and it is expected that most applications will need to use a more
 * comprehensive font handling and text layout library in addition to
 * Cairo.
 **/
void
cairo_select_font_face (cairo_t              *cr, 
			const char           *family, 
			cairo_font_slant_t    slant, 
			cairo_font_weight_t   weight)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_select_font_face (cr->gstate, family, slant, weight);
    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_get_font_face:
 * @cr: a #cairo_t
 * 
 * Gets the current font face for a #cairo_t.
 *
 * Return value: the current font object. Can return %NULL
 *   on out-of-memory or if the context is already in
 *   an error state. This object is owned by cairo. To keep
 *   a reference to it, you must call cairo_font_reference().
 **/
cairo_font_face_t *
cairo_get_font_face (cairo_t *cr)
{
    cairo_font_face_t *font_face;

    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return NULL;

    cr->status = _cairo_gstate_get_font_face (cr->gstate, &font_face);
    CAIRO_CHECK_SANITY (cr);
    return font_face;
}

/**
 * cairo_font_extents:
 * @cr: a #cairo_t
 * @extents: a #cairo_font_extents_t object into which the results
 * will be stored.
 * 
 * Gets the font extents for the currently selected font.
 **/
void
cairo_font_extents (cairo_t              *cr, 
		    cairo_font_extents_t *extents)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_get_font_extents (cr->gstate, extents);
    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_set_font_face:
 * @cr: a #cairo_t
 * @font_face: a #cairo_font_face_t, or %NULL to restore to the default font
 *
 * Replaces the current #cairo_font_face_t object in the #cairo_t with
 * @font_face. The replaced font face in the #cairo_t will be
 * destroyed if there are no other references to it.
 **/
void
cairo_set_font_face (cairo_t           *cr,
		     cairo_font_face_t *font_face)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_set_font_face (cr->gstate, font_face);  
    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_set_font_size:
 * @cr: a #cairo_t
 * @size: the new font size, in user space units
 * 
 * Sets the current font matrix to a scale by a factor of @size, replacing
 * any font matrix previously set with cairo_set_font_size() or
 * cairo_set_font_matrix(). This results in a font size of @size user space
 * units. (More precisely, this matrix will result in the font's
 * em-square being a @size by @size square in user space.)
 **/
void
cairo_set_font_size (cairo_t *cr, double size)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_set_font_size (cr->gstate, size);
    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_set_font_matrix
 * @cr: a #cairo_t
 * @matrix: a #cairo_matrix_t describing a transform to be applied to
 * the current font.
 *
 * Sets the current font matrix to @matrix. The font matrix gives a
 * transformation from the design space of the font (in this space,
 * the em-square is 1 unit by 1 unit) to user space. Normally, a
 * simple scale is used (see cairo_set_font_size()), but a more
 * complex font matrix can be used to shear the font
 * or stretch it unequally along the two axes
 **/
void
cairo_set_font_matrix (cairo_t		    *cr,
		       const cairo_matrix_t *matrix)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_set_font_matrix (cr->gstate, matrix);
    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_get_font_matrix
 * @cr: a #cairo_t
 * @matrix: return value for the matrix
 *
 * Stores the current font matrix into @matrix. See
 * cairo_set_font_matrix().
 **/
void
cairo_get_font_matrix (cairo_t *cr, cairo_matrix_t *matrix)
{
    CAIRO_CHECK_SANITY (cr);
    _cairo_gstate_get_font_matrix (cr->gstate, matrix);
}

/**
 * cairo_text_extents:
 * @cr: a #cairo_t
 * @utf8: a string of text, encoded in utf-8
 * @extents: a #cairo_text_extents_t object into which the results
 * will be stored.
 * 
 * Gets the extents for a string of text. The extents describe a
 * user-space rectangle that encloses the "inked" portion of the text,
 * (as it would be drawn by cairo_show_text). Additionally, the
 * x_advance and y_advance values indicate the amount by which the
 * current point would be advanced by cairo_show_text.
 *
 * Note that whitespace characters do not directly contribute to the
 * size of the rectangle (extents.width and extents.height). They do
 * contribute indirectly by changing the position of non-whitespace
 * characters. In particular, trailing whitespace characters are
 * likely to not affect the size of the rectangle, though they will
 * affect the x_advance and y_advance values.
 **/
void
cairo_text_extents (cairo_t              *cr,
		    const char		 *utf8,
		    cairo_text_extents_t *extents)
{
    cairo_glyph_t *glyphs = NULL;
    int num_glyphs;
    double x, y;

    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    if (utf8 == NULL) {
	extents->x_bearing = 0.0;
	extents->y_bearing = 0.0;
	extents->width = 0.0;
	extents->height = 0.0;
	extents->x_advance = 0.0;
	extents->y_advance = 0.0;
	return;
    }

    cairo_get_current_point (cr, &x, &y);

    cr->status = _cairo_gstate_text_to_glyphs (cr->gstate, utf8,
					       x, y,
					       &glyphs, &num_glyphs);
    CAIRO_CHECK_SANITY (cr);

    if (cr->status) {
	if (glyphs)
	    free (glyphs);
	return;
    }
	
    cr->status = _cairo_gstate_glyph_extents (cr->gstate, glyphs, num_glyphs, extents);
    CAIRO_CHECK_SANITY (cr);
    
    if (glyphs)
	free (glyphs);
}

/**
 * cairo_glyph_extents:
 * @cr: a #cairo_t
 * @glyphs: an array of #cairo_glyph_t objects
 * @num_glyphs: the number of elements in @glyphs
 * @extents: a #cairo_text_extents_t object into which the results
 * will be stored
 * 
 * Gets the extents for an array of glyphs. The extents describe a
 * user-space rectangle that encloses the "inked" portion of the
 * glyphs, (as they would be drawn by cairo_show_glyphs).
 * Additionally, the x_advance and y_advance values indicate the
 * amount by which the current point would be advanced by
 * cairo_show_glyphs.
 * 
 * Note that whitespace glyphs do not contribute to the size of the
 * rectangle (extents.width and extents.height).
 **/
void
cairo_glyph_extents (cairo_t                *cr,
		     cairo_glyph_t          *glyphs, 
		     int                    num_glyphs,
		     cairo_text_extents_t   *extents)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_glyph_extents (cr->gstate, glyphs, num_glyphs,
					      extents);
    CAIRO_CHECK_SANITY (cr);
}

void
cairo_show_text (cairo_t *cr, const char *utf8)
{
    cairo_glyph_t *glyphs = NULL;
    int num_glyphs;
    double x, y;

    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    if (utf8 == NULL)
	return;

    cairo_get_current_point (cr, &x, &y);

    cr->status = _cairo_gstate_text_to_glyphs (cr->gstate, utf8,
					       x, y,
					       &glyphs, &num_glyphs);
    CAIRO_CHECK_SANITY (cr);

    if (cr->status) {
	if (glyphs)
	    free (glyphs);
	return;
    }

    cr->status = _cairo_gstate_show_glyphs (cr->gstate, glyphs, num_glyphs);
    CAIRO_CHECK_SANITY (cr);

    if (glyphs)
	free (glyphs);
}

void
cairo_show_glyphs (cairo_t *cr, cairo_glyph_t *glyphs, int num_glyphs)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_show_glyphs (cr->gstate, glyphs, num_glyphs);
    CAIRO_CHECK_SANITY (cr);
}

void
cairo_text_path  (cairo_t *cr, const char *utf8)
{
    cairo_glyph_t *glyphs = NULL;
    int num_glyphs;
    double x, y;

    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cairo_get_current_point (cr, &x, &y);

    cr->status = _cairo_gstate_text_to_glyphs (cr->gstate, utf8,
					       x, y,
					       &glyphs, &num_glyphs);
    CAIRO_CHECK_SANITY (cr);

    if (cr->status) {
	if (glyphs)
	    free (glyphs);
	return;
    }

    cr->status = _cairo_gstate_glyph_path (cr->gstate,
					   glyphs, num_glyphs,
					   &cr->path);
    CAIRO_CHECK_SANITY (cr);

    if (glyphs)
	free (glyphs);
}

void
cairo_glyph_path (cairo_t *cr, cairo_glyph_t *glyphs, int num_glyphs)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    cr->status = _cairo_gstate_glyph_path (cr->gstate,
					   glyphs, num_glyphs,
					   &cr->path);
    
    CAIRO_CHECK_SANITY (cr);
}

/**
 * cairo_get_operator:
 * @cr: a cairo context
 * 
 * Gets the current compositing operator for a cairo context.
 * 
 * Return value: the current compositing operator.
 **/
cairo_operator_t
cairo_get_operator (cairo_t *cr)
{
    CAIRO_CHECK_SANITY (cr);
    return _cairo_gstate_get_operator (cr->gstate);
}

/**
 * cairo_get_tolerance:
 * @cr: a cairo context
 * 
 * Gets the current tolerance value, as set by cairo_set_tolerance().
 * 
 * Return value: the current tolerance value.
 **/
double
cairo_get_tolerance (cairo_t *cr)
{
    CAIRO_CHECK_SANITY (cr);
    return _cairo_gstate_get_tolerance (cr->gstate);
}

/**
 * cairo_get_current_point:
 * @cr: a cairo context
 * @x: return value for X coordinate of the current point
 * @y: return value for Y coordinate of the current point
 * 
 * Gets the current point of the current path, which is
 * conceptually the final point reached by the path so far.
 *
 * The current point is returned in the user-space coordinate
 * system. If there is no defined current point then @x and @y will
 * both be set to 0.0.
 *
 * Most path construction functions alter the current point. See the
 * following for details on how they affect the current point:
 *
 * cairo_new_path(), cairo_move_to(), cairo_line_to(),
 * cairo_curve_to(), cairo_arc(), cairo_rel_move_to(),
 * cairo_rel_line_to(), cairo_rel_curve_to(), cairo_arc(),
 * cairo_text_path(), cairo_stroke_to_path()
 **/
void
cairo_get_current_point (cairo_t *cr, double *x_ret, double *y_ret)
{
    cairo_status_t status;
    cairo_fixed_t x_fixed, y_fixed;
    double x, y;

    CAIRO_CHECK_SANITY (cr);

    status = _cairo_path_fixed_get_current_point (&cr->path, &x_fixed, &y_fixed);
    if (status == CAIRO_STATUS_NO_CURRENT_POINT) {
	x = 0.0;
	y = 0.0;
    } else {
	x = _cairo_fixed_to_double (x_fixed);
	y = _cairo_fixed_to_double (y_fixed);
	_cairo_gstate_backend_to_user (cr->gstate, &x, &y);
    }

    if (x_ret)
	*x_ret = x;
    if (y_ret)
	*y_ret = y;

    CAIRO_CHECK_SANITY (cr);
}
slim_hidden_def(cairo_get_current_point);

/**
 * cairo_get_fill_rule:
 * @cr: a cairo context
 * 
 * Gets the current fill rule, as set by cairo_set_fill_rule().
 * 
 * Return value: the current fill rule.
 **/
cairo_fill_rule_t
cairo_get_fill_rule (cairo_t *cr)
{
    CAIRO_CHECK_SANITY (cr);
    return _cairo_gstate_get_fill_rule (cr->gstate);
}

/**
 * cairo_get_line_width:
 * @cr: a cairo context
 * 
 * Gets the current line width, as set by cairo_set_line_width().
 * 
 * Return value: the current line width, in user-space units.
 **/
double
cairo_get_line_width (cairo_t *cr)
{
    CAIRO_CHECK_SANITY (cr);
    return _cairo_gstate_get_line_width (cr->gstate);
}

/**
 * cairo_get_line_cap:
 * @cr: a cairo context
 * 
 * Gets the current line cap style, as set by cairo_set_line_cap().
 * 
 * Return value: the current line cap style.
 **/
cairo_line_cap_t
cairo_get_line_cap (cairo_t *cr)
{
    CAIRO_CHECK_SANITY (cr);
    return _cairo_gstate_get_line_cap (cr->gstate);
}

/**
 * cairo_get_line_join:
 * @cr: a cairo context
 * 
 * Gets the current line join style, as set by cairo_set_line_join().
 * 
 * Return value: the current line join style.
 **/
cairo_line_join_t
cairo_get_line_join (cairo_t *cr)
{
    CAIRO_CHECK_SANITY (cr);
    return _cairo_gstate_get_line_join (cr->gstate);
}

/**
 * cairo_get_miter_limit:
 * @cr: a cairo context
 * 
 * Gets the current miter limit, as set by cairo_set_miter_limit().
 * 
 * Return value: the current miter limit.
 **/
double
cairo_get_miter_limit (cairo_t *cr)
{
    CAIRO_CHECK_SANITY (cr);
    return _cairo_gstate_get_miter_limit (cr->gstate);
}

/**
 * cairo_get_matrix:
 * @cr: a cairo context
 * @matrix: return value for the matrix
 *
 * Stores the current transformation matrix (CTM) into @matrix.
 **/
void
cairo_get_matrix (cairo_t *cr, cairo_matrix_t *matrix)
{
    CAIRO_CHECK_SANITY (cr);
    _cairo_gstate_get_matrix (cr->gstate, matrix);
}

/**
 * cairo_get_target:
 * @cr: a cairo context
 * 
 * Gets the target surface for the cairo context as passed to
 * cairo_create().
 * 
 * Return value: the target surface, (or NULL if @cr is in an error
 * state). This object is owned by cairo. To keep a reference to it,
 * you must call cairo_pattern_reference().
 **/
cairo_surface_t *
cairo_get_target (cairo_t *cr)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return NULL;

    return _cairo_gstate_get_target (cr->gstate);
}

/**
 * cairo_copy_path:
 * @cr: a cairo context
 * 
 * Creates a copy of the current path and returns it to the user as a
 * #cairo_path_t. See #cairo_path_data_t for hints on how to iterate
 * over the returned data structure.
 * 
 * Return value: the copy of the current path. The caller owns the
 * returned object and should call cairo_path_destroy() when finished
 * with it.
 **/
cairo_path_t *
cairo_copy_path (cairo_t *cr)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return &_cairo_path_nil;

    return _cairo_path_data_create (&cr->path, cr->gstate);
}

/**
 * cairo_copy_path_flat:
 * @cr: a cairo context
 * 
 * Gets a flattened copy of the current path and returns it to the
 * user as a #cairo_path_t. See #cairo_path_data_t for hints on
 * how to iterate over the returned data structure.
 *
 * This function is like cairo_copy_path() except that any curves
 * in the path will be approximated with piecewise-linear
 * approximations, (accurate to within the current tolerance
 * value). That is, the result is guaranteed to not have any elements
 * of type CAIRO_PATH_CURVE_TO which will instead be replaced by a
 * series of CAIRO_PATH_LINE_TO elements.
 * 
 * Return value: the copy of the current path. The caller owns the
 * returned object and should call cairo_path_destroy() when finished
 * with it.
 **/
cairo_path_t *
cairo_copy_path_flat (cairo_t *cr)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return &_cairo_path_nil;

    return _cairo_path_data_create_flat (&cr->path, cr->gstate);
}

/**
 * cairo_append_path:
 * @cr: a cairo context
 * @path: path to be appended
 * 
 * Append the @path onto the current path. See #cairo_path_t
 * for details on how the path data structure must be initialized.
 **/
void
cairo_append_path (cairo_t	*cr,
		   cairo_path_t *path)
{
    CAIRO_CHECK_SANITY (cr);
    if (cr->status)
	return;

    if (path == NULL || path->data == NULL) {
	cr->status = CAIRO_STATUS_NULL_POINTER;
	return;
    }

    if (path == &_cairo_path_nil) {
	cr->status = CAIRO_STATUS_NO_MEMORY;
	return;
    }

    cr->status = _cairo_path_data_append_to_context (path, cr);

    CAIRO_CHECK_SANITY (cr);
}

cairo_status_t
cairo_status (cairo_t *cr)
{
    CAIRO_CHECK_SANITY (cr);
    return cr->status;
}

const char *
cairo_status_string (cairo_t *cr)
{
    switch (cr->status) {
    case CAIRO_STATUS_SUCCESS:
	return "success";
    case CAIRO_STATUS_NO_MEMORY:
	return "out of memory";
    case CAIRO_STATUS_INVALID_RESTORE:
	return "cairo_restore without matching cairo_save";
    case CAIRO_STATUS_INVALID_POP_GROUP:
	return "cairo_pop_group without matching cairo_push_group";
    case CAIRO_STATUS_NO_CURRENT_POINT:
	return "no current point defined";
    case CAIRO_STATUS_INVALID_MATRIX:
	return "invalid matrix (not invertible)";
    case CAIRO_STATUS_NO_TARGET_SURFACE:
	return "no target surface has been set";
    case CAIRO_STATUS_NULL_POINTER:
	return "NULL pointer";
    case CAIRO_STATUS_INVALID_STRING:
	return "input string not valid UTF-8";
    case CAIRO_STATUS_INVALID_PATH_DATA:
	return "input path data not valid";
    case CAIRO_STATUS_READ_ERROR:
	return "error while reading from input stream";
    case CAIRO_STATUS_WRITE_ERROR:
	return "error while writing to output stream";
    case CAIRO_STATUS_SURFACE_FINISHED:
	return "the target surface has been finished";
    case CAIRO_STATUS_SURFACE_TYPE_MISMATCH:
	return "the surface type is not appropriate for the operation";
    case CAIRO_STATUS_BAD_NESTING:
	return "drawing operations interleaved for two contexts for the same surface";
    }

    return "<unknown error status>";
}

void
_cairo_restrict_value (double *value, double min, double max)
{
    if (*value < min)
	*value = min;
    else if (*value > max)
	*value = max;
}
