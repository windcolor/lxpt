/**
 * README for Availability Calendar module.
 *
 * @author Erwin Derksen (fietserwin: http://drupal.org/user/750928)
 * @link: http://drupal.org/project/availability_calendars (project page)
 *
 * Note that the module name is availability_calendar, thus without an s, though
 * the project page and the tar.gz do have an s. This is for historical reasons.
 */

The availability calendar module defines an availability calendar field. This
version is based on the fields feature of D7.

This module focuses on displaying availability. It does not try to support the
booking or payment process. Having said this, the "booking formlet" sub-module
does give you a start by offering a booking request form where visitors can
easily select their dates. But there is no further integration. So, the booking
formlet will not automatically change the availability state of a calendar. You
will have to edit the availability state manually.

If you are looking for a fully integrated booking and payment process, try the
Rooms module (http://drupal.org/project/rooms).


Dependencies
------------
- The 'booking formlet display settings form' uses OR #states which are
  available as of Drupal 7.14.
- The Views support part uses the format_string function that is available as of
  Drupal 7.9.
- The Views support part uses the date_popup if available, but this is no hard
  dependency.
- Date parsing based on (localizable) date types uses either the date_api module
  or PHP 5.3 functionality. Thus either you are on PHP 5.3 or higher, or you
  must install and enable the date_api module.


Issues in core and other modules you may run into
-------------------------------------------------
@todo: add explanation under which circumstances
- [#750928]
- [#838096]
- [#1342874]
- [#1580700]
- [#1592688]


Installing
----------
As usual. After enabling the module:
- Define the states you want to use on
  admin/config/content/availability-calendar/settings
- Define the date formats you want to use on admin/config/regional/date-time.
  You can localize these formats in admin/config/regional/date-time/locale.
- Define the basic styling, including the colors for the states, on
  admin/config/content/availability-calendar/styling
- Add availability calendar fields to the requested content types.


Upgrading from Availability Calendars 7.x-3.x
---------------------------------------------
Read the compatibility notes in CHANGELOG.txt to see what you have to check and
test.


Upgrading from Availability Calendars 7.x-2.x or earlier
--------------------------------------------------------
To Drupal this is a different module from the already existing Availability
Calendars module. This makes upgrading via update.php a bit tricky. Therefore, a
separate update module has been created. This module can be found in the latest
7.x-2.x package. So install that version as well. The Availability Calendars
update module contains an UPGRADE.txt with more detailed information about
upgrading.


Styling
-------
The modules contains a style sheet with basic styling that should give you a
reasonable look & feel out of the box (availability_calendar.base.css).
Additionally, you can specify some styling via the admin user interface on
admin/config/content/availability-calendar/styling. This will generate a file
sites/default/files/availability_calendar/availability_calendar.css. Remaining
styling is to be defined in your theme. See availability_calendar.base.css for
how the calendar and key are rendered.


Views integration
-----------------
Views integration has been added. Note that although there is a separate filter
on the calendar option "enabled" in the views UI, if you define a view that
accesses information from one of the availability_calendar_* tables, an extra
join condition on enabled will be added automatically. So normally there is no
need to add this filter, except perhaps in some administrative edge use cases.


Search on availability
----------------------
Through the views integration it is now possible to search on availability. Just
add a filter "<field name> available". The end date that is filled in, probably
exposed to the visitor, is either inclusive (with the 'From begin up to and
including end' operator) or not inclusive (with the 'From arrival to departure'
operator).


Caching
-------
Caching pages with availability calendars is possible but keep in mind that the
calendars change just because the date changes, thus without anyone changing the
data that belongs to the calendar. This means that ideally you should set your
page caches to expire next midnight. However, most caching mechanisms, including
the standard one provided by Drupal, only allows you to set an offset to the
current time. So an offset up to half a day should not give you many problems.
Note that in a multilingual set-up with field syncing (i18n_sync module) field
syncing goes through node_save and thus invalidates the cache.


I18n
----
Availability calendar is (or strives to be) fully multilingual aware. Using the
standard translation model - several entities composing 1 translation set - the
calendars can be shared between translations by enabling field syncing for them.

The names of the states are considered hard coded texts and thus translated
using t() not i18n_string, even though they may be overridden via user entered
input. They should thus be entered in English.

The names of the calendars are field values and thus not translated. On syncing
they won't overwrite already existing names, but if no name exists in the target
language the name is copied.

form labels are passed through t() and thus can be translated. If you want to
change the labels completely because, e.g., the terms arrival and departure do
not fit well in your use case, you can use the String Overrides module
(http://drupal.org/project/stringoverrides). This might get changed in the
future by placing these texts in variables (that can be made multilingual aware
with i18n_variable).


API
---
All database access, querying as well as writing, is placed in separate
functions, thus never directly in form handling functions. So this functionality
is easily available to other modules. To make use of the API you have to include
the .inc file:
  module_load_include('inc', 'availability_calendar', 'availability_calendar');
This to prevent the API being loaded on every request.
