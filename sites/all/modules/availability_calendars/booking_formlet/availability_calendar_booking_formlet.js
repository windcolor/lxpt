(function($) {
"use strict";

/**
 * @class Drupal.availabilityCalendar.BookingFormlet
 *   Provides the glueing code that connects the reservation formlet with the
 *   availability calendar fields on the page.
 *
 *   This class should be able to operate on its own if no calendar is loaded
 *   on the page, though availability_calendars.js remains needed for some
 *   helper functions.
 *
 * @param {Object} settings
 * @param {String} settings.formId
 *   The id of the form(let) instance that this object instance is linked to.
 *   The id has to be prefixed with a '#', so it can serve as a jQuery selector.
 * @param {Integer[]} settings.cids
 *   The list of calendar id's to listen to.
 * @param {Integer} settings.bookedState
 *   The state id (sid) to visually change the state to after the user has
 *   clicked both an arrival and departure date. This gives the visitor visual
 *   feedback and suggest that the state changes to "optionally booked".
 * @param {Boolean} settings.singleDay
 *   true if this formlet should accept only 1 date (for both start and end
 *   date), false if it should accept different dates.
 *
 * @see AvailabilityCalendars API object.
 */
Drupal.availabilityCalendar.BookingFormlet = function(settings) {
  var that = this;
  var cid;
  /** @type {Drupal.availabilityCalendar.Calendar} */
  var calendar;
  var bookingFormletSettings;
  /** @type {Date} */
  var from;
  /** @type {Date} */
  var to;
  var calSelectedDay;

  /**
   * Initializes this object.
   */
  this.init = function(settings) {
    bookingFormletSettings = settings;
    $(bookingFormletSettings.formId).once("Drupal-availabilityCalendar-BookingFormlet", function () {
      // Check cids:
      if (Drupal.availabilityCalendar.getCalendar === undefined) {
        bookingFormletSettings.cids = [];
      }
      // Extract values from form elements if preset by the server.
      cid = $('[name="cid"]', bookingFormletSettings.formId).val();
      if (cid === '') {
        cid = null;
      }
      calendar = cid !== null ? Drupal.availabilityCalendar.getCalendar(cid) : null;
      from  = null;
      to = null;
      calSelectedDay = null;

      // from and to are empty or in ISO format (yyyy-mm-dd).
      var fromIso = parseIsoDate($('[name="from_iso"]', bookingFormletSettings.formId).val());
      var toIso = parseIsoDate($('[name="to_iso"]', bookingFormletSettings.formId).val());

      // Show form in correct state. If we have preset values we use the addDate
      // event handler to also make changes to the calendar.
      if (cid !== null && fromIso !== null) {
        that.addDate(null, fromIso, cid);
        if (toIso !== null && !bookingFormletSettings.singleDay) {
          if (calendar.isOvernight()) {
            toIso.setDate(toIso.getDate() + 1);
          }
          that.addDate(null, toIso, cid);
        }
      }
      else {
        show();
      }

      // Attach to the calendar click from the calendar controller.
      var i, calendar1;
      for (i = 0; i < bookingFormletSettings.cids.length; i++) {
        calendar1 = Drupal.availabilityCalendar.getCalendar(bookingFormletSettings.cids[i]);
        if (calendar1 !== null) {
          $(calendar1).bind("calendarClick", that.addDate);
        }
      }

      // Attach to the click events of the reset buttons.
      $(".acbf-reset-from", bookingFormletSettings.formId).click(that.resetFrom);
      $(".acbf-reset-both", bookingFormletSettings.formId).click(that.resetBoth);
    });
  };

  /**
   * Helper function to parse a date string in ISO format (yyyy-mm-dd).
   *
   * @param {String} date
   * @returns {?Date}
   */
  function parseIsoDate(date) {
    var result;
    if (date) {
      result = new Date(date.substr(0, 4), date.substr(5, 2) - 1, date.substr(8, 2));
      if (isNaN(result.getTime())) {
        result = null;
      }
    }
    else {
      result = null;
    }
    return result;
  }

  /**
   * Adds a date to the command.
   *
   * - If it is the first date, it will be the from date.
   * - If it is the 2nd date, it will be the to date, swapping the from and to
   *   dates if needed.
   * - If it is a 3rd date, either the from or to date will be changed,
   *   depending on whether the 3rd date is before the current from or not.
   *
   * @param {jQuery.Event} event
   *   The event object.
   * @param {Date} date
   *   The clicked date.
   * @param {Integer} eventCid
   *   The id of the calendar where the click originated from.
   * @param {Drupal.availabilityCalendar.View} [eventView]
   *   The id of the calendar where the click originated from.
   */
  this.addDate = function(event, date, eventCid, eventView) {
    // Create a clone.
    date = new Date(date.getTime());
    // Does this event came from a different calendar on the page?
    if (cid !== eventCid) {
      // Reset the old calendar, both the settings and visually.
      that.resetBoth();
      show();
      // Assign info from the new calendar and view to the internal members.
      cid = eventCid;
      calendar = Drupal.availabilityCalendar.getCalendar(eventCid);

      $('[name="cid"]', bookingFormletSettings.formId).val(cid);
      if (eventView) {
        $('[name="calendar_label"]', bookingFormletSettings.formId).val(eventView.getName());
      }
    }

    if (to !== null) {
      // Change of range: reset the currently selected range.
      calendar.restoreRangeState(from, to);
    }

    if (bookingFormletSettings.singleDay) {
      // Assign the same date but different instances to both from and to.
      from = date;
      to = new Date(date.getTime());
    }
    else {
      if (from === null) {
        // 1st date: assign to from.
        from = date;
      }
      else {
        // 2nd date. Assign to from or to depending on order.
        if (date >= from) {
          // 2nd date is after the 1st date: assign to to.
          to = date;
        }
        else {
          // 2nd date is before the 1st date: assign to from.
          // If to does not already have a value it gets the from date assigned,
          // otherwise it remains unchanged.
          to = to || from;
          from = date;
        }
      }
    }
    show();
  };

  /**
   * Resets the from date and restores the calendar.
   */
  this.resetFrom = function() {
    from = null;
    if (bookingFormletSettings.singleDay) {
      to = null;
    }
    show();
    // Stop propagating the event.
    return false;
  };

  /**
   * Resets both dates and restores the calendar.
   */
  this.resetBoth = function() {
    if (from !== null && to !== null) {
      calendar.restoreRangeState(from, to);
    }
    from = to = null;
    show();
    // Stop propagating the event.
    return false;
  };

  /**
   * @returns Boolean Whether the current form values are valid.
   */
  this.isValid = function() {
    return to !== null && from !== null && cid !== null;
  };

  /**
   * Shows the current values, help texts, and enables the submit button.
   */
  function show() {
    if (from === null) {
      // No dates:
      // - Remove the "selected state" from the calendar.
      removeCalSelected();
      // - Hide reset buttons.
      $(".form-reset", bookingFormletSettings.formId).css("display", "none");
      // - Disable arrival date field and set help text in it.
      $('[name="from_display"]', bookingFormletSettings.formId)
        .attr("disabled", "disabled")
        .addClass("form-button-disabled")
        .val(Drupal.t("Click on an available date in the calendar"));
      // - Disable and empty departure date field.
      $('[name="to_display"]', bookingFormletSettings.formId)
        .val("")
        .attr("disabled", "disabled")
        .addClass("form-button-disabled");
      // - Reset iso dates.
      $('[name="from_iso"]', bookingFormletSettings.formId).val("");
      $('[name="to_iso"]', bookingFormletSettings.formId).val("");
    }
    else {
      // 1 or 2 dates set:
      // - Set value in arrival date field and remove its disabled attribute.
      $('[name="from_display"]', bookingFormletSettings.formId)
        .val(Drupal.availabilityCalendar.formatDate(from))
        .removeAttr("disabled")
        .removeClass("form-button-disabled");
      // Set iso from date.
      $('[name="from_iso"]', bookingFormletSettings.formId).val(Drupal.availabilityCalendar.formatIsoDate(from));
      if (to === null) {
        // 1 date only:
        // - Show "clear arrival date" button, hide "Clear both dates" button.
        $(".acbf-reset-from", bookingFormletSettings.formId).css("display", "inline-block");
        $(".acbf-reset-both", bookingFormletSettings.formId).css("display", "none");
        // - Disable departure date field and set help text in it.
        $('[name="to_display"]', bookingFormletSettings.formId)
          .val(Drupal.t("Click on your departure date"))
          .attr("disabled", "disabled")
          .addClass("form-button-disabled");
        // - Reset iso to date.
        $('[name="to_iso"]', bookingFormletSettings.formId).val("");
        // - Set this single date to the "selected state".
        setCalSelected(from);
      }
      else {
        // 2 dates set:
        // - Remove the "selected state" from the calendar.
        removeCalSelected();
        // - Set iso to date.
        //
        // In day rental this will be the same as the clicked date (= to).
        // In overnight rental situations people click on the departure date and
        // the iso to date will be set to the day before.
        // But in the case of clicking twice on the same day, it will be handled
        // as a 1 day/night booking, even in the overnight rental situation.
        var toIso = new Date(to.getTime());
        if (calendar.isOvernight()) {
          if (to > from) {
            // Adjust isoTo by going back 1 day.
            toIso.setDate(toIso.getDate() - 1);
          }
          else {
            // Clicked twice on the same day: handle as if the 2nd click was on
            // the next day.
            to.setDate(to.getDate() + 1);
          }
        }
        $('[name="to_iso"]', bookingFormletSettings.formId).val(Drupal.availabilityCalendar.formatIsoDate(toIso));
        // - Set value in departure date field and remove disabled attribute.
        $('[name="to_display"]', bookingFormletSettings.formId)
          .val(Drupal.availabilityCalendar.formatDate(to))
          .removeAttr("disabled")
          .removeClass("form-button-disabled");
        // - Hide "clear arrival date" button, show  "Clear both dates" button.
        $(".acbf-reset-from", bookingFormletSettings.formId).css("display", "none");
        $(".acbf-reset-both", bookingFormletSettings.formId).css("display", "inline-block");
        // - Visually update the calendar.
        if (calendar !== null && that.isValid()) {
          calendar.changeRangeState(from, toIso, bookingFormletSettings.bookedState);
        }
      }
    }
    // Enable or disable the submit button depending on the validity of the form.
    if (that.isValid()) {
      $(".form-submit", bookingFormletSettings.formId).removeAttr("disabled").removeClass("form-button-disabled");
    }
    else {
      $(".form-submit", bookingFormletSettings.formId).attr("disabled", "disabled").addClass("form-button-disabled");
    }

  }

  /**
   * Marks the given day as selected (by adding the class cal-selected).
   *
   * @param {Date} day
   */
  function setCalSelected(day) {
    if (calendar !== null) {
      removeCalSelected();
      calSelectedDay = new Date(day.getTime());
      calendar.addExtraState(calSelectedDay, "cal-selected");
    }
  }

  /**
   * Removes the selected mark form the day it was previously set on.
   */
  function removeCalSelected() {
    if (calSelectedDay !== null && calendar !== null) {
      calendar.removeExtraState(calSelectedDay, "cal-selected");
      calSelectedDay = null;
    }
  }

  this.init(settings);
};

/**
 * @type {Object} Collection of booking formlet instances.
 */
Drupal.availabilityCalendar.bookingFormlets = {};

/**
 * Multiton implementation for accessing booking formlets on the page.
 *
 * @param {String} formId
 * @return {?Drupal.availabilityCalendar.BookingFormlet}
 */
Drupal.availabilityCalendar.getBookingFormlet = function(formId) {
  return Drupal.availabilityCalendar.bookingFormlets[formId] !== undefined  ? Drupal.availabilityCalendar.bookingFormlets[formId] : null;
};

/**
 * Initialization code that works by implementing the Drupal behaviors paradigm.
 *
 * Based on the information in the settings, the booking formlets are created.
 */
Drupal.behaviors.availabilityCalendarBookingFormlet = {
  attach: function(context, settings) {
    if (settings.availabilityCalendar.bookingFormlets) {
      var key;
      for (key in settings.availabilityCalendar.bookingFormlets) {
        if (settings.availabilityCalendar.bookingFormlets.hasOwnProperty(key)) {
          var bookingFormletSettings = settings.availabilityCalendar.bookingFormlets[key];
          if (Drupal.availabilityCalendar.bookingFormlets[bookingFormletSettings.formId] === undefined) {
            Drupal.availabilityCalendar.bookingFormlets[bookingFormletSettings.formId] = new Drupal.availabilityCalendar.BookingFormlet(bookingFormletSettings);
          }
          else {
            Drupal.availabilityCalendar.bookingFormlets[bookingFormletSettings.formId].init(bookingFormletSettings);
          }
        }
      }
    }
  }
};

}(jQuery));
