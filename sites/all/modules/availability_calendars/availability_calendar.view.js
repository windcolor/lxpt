(function($) {
"use strict";

/**
 * Javascript API for Availability Calendar field module.
 *
 * @class Drupal.availabilityCalendar.View
 *   Represents a client-side view for a given availability calendar. It is
 *   possible to have multiple calendars on the same page and each calendar can
 *   have multiple views attached. The cvid parameter is used to distinguish
 *   calendar views.
 *
 * @constructor
 *   Creates a new availability calendar view object.
 * @param {Object} settings
 * @param {Integer} settings.cvid
 *   The id for this calendar view.
 * @param {Integer} settings.cid
 *   The calendar id for the calendar we want to interact with.
 *   A {String} starting with 'new' for not yet existing calendars.
 * @param {String} settings.name
 *   The (localized) name of this calendar.
 * @param {Boolean} settings.splitDay
 *   Indicates whether this calendar should be visualized using split days.
 * @param {String} settings.selectMode
 *   none|all|available|not-available
 *   Indicates whether this calendar allows interaction by selecting dates,
 *   and if so, what states may be selected
 */
Drupal.availabilityCalendar.View = function(settings) {
  var that = this;
  /** @type {String} The (jQuery) selector for this calendar view. */
  var viewSelector;
  /** @type {Drupal.availabilityCalendar.Calendar} */
  var calendar;
  var viewSettings;
  var cells;

  /**
   * Initializes the calendar:
   * - Gives selectable cells the class selectable
   * - Initializes the custom calendarClick event on these cells
   */
  this.init = function (settings) {
    viewSettings = settings;
    viewSelector = "#cal-view-" + viewSettings.cvid;
    calendar =  Drupal.availabilityCalendar.getCalendar(viewSettings.cid);
    initDaysAdministration();
    calendar.attachView(that);
    initSelectable();
    initEvents();
  };

  /**
   * Creates a list of all days and their DOM element.
   */
  function initDaysAdministration() {
    cells = {};
    var firstMonth = true;
    // Get all calendar months.
    $(".cal-month[data-cal-month]", $(viewSelector))
      .each(function() {
        // Get year and month of this calendar month.
        var day = null;
        var year = $(this).attr("data-cal-year");
        var month = $(this).attr("data-cal-month");
        var yearMonth = year + "-" + month + "-";
        // Get all day cells of this calendar month.
        $("tbody td", $(this))
          .not(".cal-other, .cal-pastdate, .cal-empty")
          .each(function() {
            // Using Number(cell.text()) is bad for performance.
            // Using the fact that the set is ordered we only have to use it for
            // the first selected cell in the first month (as for other months
            // it will be day 1).
            // http://docs.jquery.com/Release:jQuery_1.3.2#Elements_Returned_in_Document_Order
            day = day !== null ? day + 1 :
              firstMonth ? Number(this.textContent || $(this).text()) :
              1;

            var extraStates = [];
            // Split the class property in separate classes.
            $.each(this.className.split(/\s+/), function (i, cssClass) {
              // Ignore classes ending with -am, -pm and defined states.
              if (cssClass.substr(cssClass.length - 3) !== "-am" &&
                  cssClass.substr(cssClass.length - 3) !== "-pm" &&
                  Drupal.availabilityCalendar.getStates().get(cssClass) === null) {
                extraStates.push(cssClass);
              }
            });

            cells[yearMonth + (day < 10 ? "0" : "") + day] = {cell: this, extraStates: extraStates};
          });
          firstMonth = false;
      });
  }

  /**
   * Makes certain cells selectable. Note that on overnight calendars, people
   * click on the departure date, a date that itself may be unavailable.
   */
  function initSelectable() {
    if (viewSettings.selectMode !== "none") {
      var isPreviousSelectable = false;
      var day;
      for (day in cells) {
        if (cells.hasOwnProperty(day)) {
          if (viewSettings.selectMode === "all" ||
              (viewSettings.selectMode === "available" && calendar.isAvailable(day)) ||
              (viewSettings.selectMode === "not-available" && !calendar.isAvailable(day))) {
            that.addExtraState(null, day, "cal-selectable");
            isPreviousSelectable = true;
          }
          else {
            if (isPreviousSelectable && calendar.isOvernight()) {
              that.addExtraState(null, day, "cal-selectable");
            }
            isPreviousSelectable = false;
          }
        }
      }
    }
  }

  /**
   * Initializes the event handling for this calendar.
   *
   * Currently, we react to clicking on a day cell in the calendar, and we bind
   * to the state changing events from Drupal.availabilityCalendar.Calendar.
   */
  function initEvents() {
    // Bind to the events defined by the calendar controller.
    $(calendar).bind("stateChange", that.changeState)
      .bind("extraStateAdd", that.addExtraState)
      .bind("extraStateRemove", that.removeExtraState);

    // Bind to click events on the cells.
    $(viewSelector).click(function(event) {
      // Find out if event originated from a day cell, get the date,
      // and inform the calendar controller.
      var day, month, year;
      var cell = $(event.target).closest("td.cal-selectable");
      if (cell.size() > 0) {
        cell
          .each(function() {
            day = Number($(this).text());
          })
          .closest(".cal-month[data-cal-month]")
          .each(function() {
            year = $(this).attr("data-cal-year");
            month = $(this).attr("data-cal-month");
            calendar.selectDate(new Date(year, month - 1, day), that);
          });
      }
    });
  }

  /**
   * @returns {Integer}
   *   The id of the calendar view.
   */
  this.getCvid = function() {
    return viewSettings.cvid;
  };

  /**
   * @returns {String}
   *   The name of the calendar.
   */
  this.getName = function() {
    return viewSettings.name;
  };

  /**
   * @returns {Drupal.availabilityCalendar.Calendar}
   *   The calendar that this view represents.
   */
  this.getCalendar = function() {
    return calendar;
  };

  /**
   * @returns {jQuery}
   *   The calendar view element.
   */
  this.getView = function() {
    return $(viewSelector);
  };

  /**
   * @returns {Boolean}
   *   If the calendar is shown using split days.
   */
  this.isSplitDay = function() {
    return calendar.isOvernight() && viewSettings.splitDay;
  };

  /**
   * @returns {Object}
   *   A list of calendar cells indexed by their date.
   */
  this.getCells = function() {
    return cells;
  };

  /**
   * @returns {Integer}
   *   The number of months in the current calendar view.
   */
  this.getNumberOfMonths = function() {
    return $(".cal-month", $(viewSelector)).length;
  };

  /**
   * Internal function to combine the state and any extra states
   * for the given day to 1 value for the className property.
   *
   * @param {String} day
   */
  function setCellClass(day) {
    var state = calendar.getState(day);
    var cssClasses = [];
    if (that.isSplitDay()) {
      var yesterday = new Date(day.substr(0, 4), day.substr(5, 2) - 1, day.substr(8, 2));
      yesterday.setDate(yesterday.getDate() - 1);
      var stateYesterday = calendar.getState(yesterday);
      if (stateYesterday !== "") {
        cssClasses.push(stateYesterday + "-am");
      }
      if (state !== "") {
        cssClasses.push(state + "-pm");
      }
    }
    else {
      if (state !== "") {
        cssClasses.push(state);
      }
    }
    cssClasses = cssClasses.concat(cells[day].extraStates);
    cells[day].cell.className = cssClasses.join(" ");
  }

  /**
   * Event handler that reacts to a stateChange event of a calendar controller.
   *
   * @param {Event} event
   * @param {Date} day
   * param {Integer} cid
   */
  this.changeState = function(event, day/*, cid*/) {
    var date = Drupal.availabilityCalendar.formatIsoDate(day);
    setCellClass(date);
    if (that.isSplitDay()) {
      var tomorrow = new Date(day.getTime());
      tomorrow.setDate(tomorrow.getDate() + 1);
      tomorrow = Drupal.availabilityCalendar.formatIsoDate(tomorrow);
      setCellClass(tomorrow);
    }
  };

  /**
   * Sets an extra state on the given day.
   *
   * Extra states do not mix with or replace the availability settings.
   * An extra state is not added twice.
   *
   * @param {Event} event
   * @param {Date|String} day
   * @param {String} extraState
   */
  this.addExtraState = function(event, day, extraState) {
    var date = Drupal.availabilityCalendar.formatIsoDate(day);
    if (cells[date] !== undefined) {
      // Only add if extra state is not already set.
      if (cells[date].extraStates.indexOf(extraState) < 0) {
        cells[date].extraStates.push(extraState);
        setCellClass(date);
      }
    }
  };

  /**
   * Removes an extra state of the given day.
   *
   * Extra states do not mix with or replace the availability settings.
   *
   * @param {Event} event
   * @param {Date|String} day
   * @param {String} extraState
   */
  this.removeExtraState = function(event, day, extraState) {
    var date = Drupal.availabilityCalendar.formatIsoDate(day);
    if (cells[date] !== undefined) {
      // Only remove if extra state is set.
      var index = cells[date].extraStates.indexOf(extraState);
      if (index >= 0) {
        cells[date].extraStates.splice(index, 1);
        setCellClass(date);
      }
    }
  };

  /**
   * Adds the given extra state to all days in the from - to range.
   *
   * Extra states do not mix with or replace the availability settings.
   *
   * @param {Date} from
   * @param {Date} to
   * @param {String} extraState
   */
  this.addRangeExtraState = function(from, to, extraState) {
    // Loop through range of dates.
    var date = new Date(from.getTime());
    while (date <= to) {
      that.addExtraState(null, date, extraState);
      date.setDate(date.getDate() + 1);
    }
  };

  /**
   * Removes the given state from all days in the from - to range.
   *
   * Extra states do not mix with or replace the availability settings.
   *
   * @param {Date} from
   * @param {Date} to
   * @param {String} extraState
   */
  this.removeRangeExtraState = function(from, to, extraState) {
    // Loop through range of dates.
    var date = new Date(from.getTime());
    while (date <= to) {
      that.removeExtraState(null, date, extraState);
      date.setDate(date.getDate() + 1);
    }
  };

  this.init(settings);
};

/**
 * @type {Object} Collection of calendar view instances.
 */
Drupal.availabilityCalendar.views = {};

  /**
   * Multiton implementation for accessing views on the page.
   *
   * @param {Integer} cvid
   * @returns {?Drupal.availabilityCalendar.View}
   */
Drupal.availabilityCalendar.getCalendarView = function(cvid) {
  // We can only return an existing calendar,
  return Drupal.availabilityCalendar.views[cvid] !== undefined ? Drupal.availabilityCalendar.views[cvid] : null;
};

/**
 * Initialization code that works by implementing the Drupal behaviors paradigm.
 *
 * Based on the information in the settings, the calendar views are created.
 */
Drupal.behaviors.availabilityCalendarView = {
  attach: function(context, settings) {
    if (settings.availabilityCalendar.views) {
      var key;
      for (key in settings.availabilityCalendar.views) {
        if (settings.availabilityCalendar.views.hasOwnProperty(key)) {
          var viewSettings = settings.availabilityCalendar.views[key];
          if (Drupal.availabilityCalendar.views[viewSettings.cvid] === undefined) {
            Drupal.availabilityCalendar.views[viewSettings.cvid] = new Drupal.availabilityCalendar.View(viewSettings);
          }
          else {
            Drupal.availabilityCalendar.views[viewSettings.cvid].init(viewSettings);
          }
        }
      }
    }
  }
};

}(jQuery));
