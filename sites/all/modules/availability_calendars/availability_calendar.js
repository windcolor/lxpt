(function($) {
"use strict";

/*
 * IE8- hack. It turns out that IE8 and lower do not support the indexOf method
 * on arrays. Define it here.
 */
if (!Array.indexOf) {
  Array.prototype.indexOf = function (obj, start) {
    var i;
    for (i = (start || 0); i < this.length; i++) {
      if (this[i] === obj) {
        return i;
      }
    }
    return -1;
  };
}

Drupal.availabilityCalendar = Drupal.availabilityCalendar || {};

/**
 * Helper method to format a date according to the date format defined for user
 * presented dates.
 */
Drupal.availabilityCalendar.formatDate = function(date) {
  return date instanceof Date ? $.datepicker.formatDate(Drupal.settings.availabilityCalendar.displayDateFormat, date) : date;
};

/**
 * Formats a date according to the ISO 8601 date format (yyyy-mm-dd).
 *
 * @param {String|Date} date
 * @return {String}
 */
Drupal.availabilityCalendar.formatIsoDate = function(date) {
  return date instanceof Date ? $.datepicker.formatDate('yy-mm-dd', date) : date;
};

/**
 * @class Drupal.availabilityCalendar.States
 *   Represents the collection of all availability states. Per state some
 *   information is kept:
 *   - {Integer} sid,
 *   - {String} cssClass
 *   - {Boolean} isAvailable
 *
 * @constructor
 *   Creates a new collection of all availability states.
 * @param {Object[]} [states]
 *   Array with information about all possible availability states.
 */
Drupal.availabilityCalendar.States = function(states) {
  var statesById = {};
  var statesByClass = {};

  this.init = function(states) {
    var key;
    // Add empty state (easiest way to prevent javascript errors).
    states[""] = {sid: 0, cssClass: "", isAvailable: false};
    statesById = {};
    statesByClass = {};
    for (key in states) {
      if (states.hasOwnProperty(key)) {
        statesById[states[key].sid] = states[key];
        statesByClass[states[key].cssClass] = states[key];
      }
    }
  };

  /**
   * Returns the state for the given class or state id.
   *
   * @param {Integer|String} stateOrId
   * @returns {?Object}
   *   {cid:Integer, cssClass:String, isAvailable:Boolean}
   */
  this.get = function(stateOrId) {
    if (isNaN(stateOrId) && statesByClass[stateOrId] !== undefined) {
      return statesByClass[stateOrId];
    }
    else if (!isNaN(stateOrId) && statesById[stateOrId] !== undefined) {
      return statesById[stateOrId];
    }
    return null;
  };

  this.init(states);
};

/**
 * @class Drupal.availabilityCalendar.Calendar
 *   Represents the controller for a given availability calendar.
 *
 * It is possible to have multiple calendars on the same page, each will get
 * its own controller instance. The cid parameter is used to distinguish
 * calendar controllers.
 *
 * A calendar can be displayed multiple times on the same page. each display
 * will be represented by a Drupal.availabilityCalendar.CalendarView that
 * connects to a controller.
 *
 * The calendar controller class offers the following features and events:
 * - Methods to query properties from the calendar: cid, isOvernight.
 * - Methods to retrieve information about the calendar: getState, isAvailable,
 *   isRangeAvailable.
 * - Methods to change the (visual) status of calendar days. However, note that
 *   it does not update the server-side calendar: changeState, restoreState,
 *   addExtraState, removeExtraState, changeRangeState, removeRangeState
 * - getViews: get a list of connected Drupal.availabilityCalendar.CalendarView
 *   objects.
 * - Event calendarClick: triggered when the visitor clicks on a day cell in
 *   1 of the attached views. The 'calendarClick' event passes in a Date
 *   object, representing the day that was clicked on, and the cid, to
 *   identify which calendar was clicked on.
 * - selectDate: method to trigger the calendarClick event.
 *
 * @constructor
 *   Creates a new AvailabilityCalendar controller object.
 * @param {Object} settings
 * @param {Integer|String} settings.cid
 *   The calendar id for the calendar we want to interact with.
 *   A string starting with 'new' for not yet existing calendars.
 * @param {Boolean} settings.overnight
 *   Indicates whether this calendar is used for overnight or for day bookings.
 */
Drupal.availabilityCalendar.Calendar = function(settings) {
  var that = this;
  var calendarSettings;
  /**
   * @type {Object} List of states (string: CSS class) indexed by date
   *   (string: yyyy-mm-dd format).
   */
  var days = {};
  /** @type {Integer[]} List of attached view ids. */
  var views = [];

  this.init = function (settings) {
    calendarSettings = settings;
  };

  /**
   * Creates an overview of all days and their states.
   *
   * @param {Object} cells
   *   A list of cells representing the dates in the calendar range and indexed
   *   by date (string in yyyy-mm-dd format).
   */
  function initDaysAdministration(cells) {
    // Extract the state information from each cell of the view. Each cell is an
    // object with 2 properties: cell and (ignored here) extraStates.
    $.each(cells, function (date, cell) {
      // Define the date in our administration (if not already set).
      if (days[date] === undefined) {
        days[date] = [];
      }
      // Split the class property in separate classes.
      $.each(cell.cell.className.split(/\s+/), function (i, cssClass) {
        // Ignore classes ending with -am.
        if (cssClass.substr(cssClass.length - 3) !== "-am") {
          // Remove any -pm at the end.
          if (cssClass.substr(cssClass.length - 3) === "-pm") {
            cssClass = cssClass.substring(0, cssClass.length - 3);
          }
          // Only add defined states (not extra states).
          if (Drupal.availabilityCalendar.getStates().get(cssClass) !== null) {
            // Overwrite any existing state.
            days[date] = [cssClass];
          }
        }
      });
    });
  }

  /**
   * @returns {Integer} The cid of the calendar.
   */
  this.getCid = function () {
    return calendarSettings.cid;
  };

  /**
   * @returns {Boolean}
   *   If the calendar is used for overnight or for day rental.
   */
  this.isOvernight = function() {
    return calendarSettings.overnight;
  };

  /**
   * Attaches a calendar view to this controller.
   *
   * @param {Drupal.availabilityCalendar.View} calendarView
   */
  this.attachView = function(calendarView) {
    if (views.indexOf(calendarView.getCvid()) < 0) {
      views.push(calendarView.getCvid());
    }
    initDaysAdministration(calendarView.getCells());
  };

  /**
   * @returns {Integer[]} A list of ids of attached views.
   */
  this.getViews = function() {
    return views;
  };

  /**
   * Returns the state id for the given date.
   *
   * @param {Date|String} date
   * @returns {String}
   *   The CSS class for the given date, or the empty string if the date is not
   *   within the calendar range.
   */
  this.getState = function(date) {
    date = Drupal.availabilityCalendar.formatIsoDate(date);
    return days[date] !== undefined ? days[date][days[date].length - 1] : "";
  };

  /**
   * Returns whether the given day is available.
   *
   * @param {Date|String} date
   * @returns {?Boolean}
   *   true if the date is available.
   *   false if the date is not available.
   *   null if the date is not within the calendar range.
   */
  this.isAvailable = function(date) {
    var state = this.getState(date);
    return state !== "" ? Drupal.availabilityCalendar.getStates().get(state).isAvailable : null;
  };

  /**
   * Informs this calendar controller that a day has been clicked/selected.
   *
   * @param {Date} date
   * @param {Drupal.availabilityCalendar.View} calendarView
   */
  this.selectDate = function(date, calendarView) {
    var dateIso = Drupal.availabilityCalendar.formatIsoDate(date);
    if (days[dateIso] !== undefined) {
      $(that).trigger("calendarClick", [date, calendarSettings.cid, calendarView]);
    }
  };

  /**
   * Changes the availability state of the given day.
   *
   * @param {Date} date
   * @param {Integer|String} state
   */
  this.changeState = function(date, state) {
    var dateIso = Drupal.availabilityCalendar.formatIsoDate(date);
    if (days[dateIso] !== undefined) {
      days[dateIso].push(Drupal.availabilityCalendar.getStates().get(state).cssClass);
      $(that).trigger("stateChange", [date, calendarSettings.cid]);
    }
  };

  /**
   * Restores the availability state of the given day to its previous value.
   *
   * @param {Date} date
   */
  this.restoreState = function(date) {
    var dateIso = Drupal.availabilityCalendar.formatIsoDate(date);
    if (days[dateIso] !== undefined) {
      // Remove current state (if not the original state).
      if (days[dateIso].length > 1) {
        days[dateIso].pop();
        $(that).trigger("stateChange", [date, calendarSettings.cid]);
      }
    }
  };

  /**
   * Informs views to set an extra state on the given day.
   *
   * Extra states do not mix with or replace the availability settings.
   * An extra state is not added twice.
   *
   * @param {Date|String} date
   * @param {String} extraState
   */
  this.addExtraState = function(date, extraState) {
    var dateIso = Drupal.availabilityCalendar.formatIsoDate(date);
    if (days[dateIso] !== undefined) {
      $(that).trigger("extraStateAdd", [date, extraState]);
    }
  };

  /**
   * Informs views to remove an extra state on the given day.
   *
   * Extra states do not mix with or replace the availability settings.
   * An extra state is not added twice.
   *
   * @param {Date|String} date
   * @param {String} extraState
   */
  this.removeExtraState = function(date, extraState) {
    var dateIso = Drupal.availabilityCalendar.formatIsoDate(date);
    if (days[dateIso] !== undefined) {
      $(that).trigger("extraStateRemove", [date, extraState]);
    }
  };

  /**
   * Returns whether all dates in the given range are available.
   *
   * @param {Date} from
   * @param {Date} to
   * @returns {?Boolean}
   *   true if the whole range is available,
   *   false if not the whole range is available,
   *   null if the given date range is not fully within the calendar range.
   */
  this.isRangeAvailable = function(from, to) {
    var available = true;
    var date = new Date(from.getTime());
    while (available === true && date <= to) {
      available = this.isAvailable(date);
      date.setDate(date.getDate() + 1);
    }
    return available;
  };

  /**
   * Sets all days in the from - to range to the given state.
   *
   * @param {Date} from
   * @param {Date} to
   * @param {Integer|String} state
   */
  this.changeRangeState = function(from, to, state) {
    // Loop through range of dates.
    var date = new Date(from.getTime());
    while (date <= to) {
      this.changeState(date, state);
      date.setDate(date.getDate() + 1);
    }
  };

  /**
   * Restores all days in the from - to range to their previous state.
   *
   * @param {Date} from
   * @param {Date} to
   */
  this.restoreRangeState = function(from, to) {
    // Loop through range of dates.
    var date = new Date(from.getTime());
    while (date <= to) {
      this.restoreState(date);
      date.setDate(date.getDate() + 1);
    }
  };

  this.init(settings);
};

/**
 * @type {Object} Collection of calendar states.
 */
Drupal.availabilityCalendar.states = null;

/**
 * Singleton implementation for accessing calendar states.
 *
 * @returns Drupal.availabilityCalendar.States
 */
Drupal.availabilityCalendar.getStates = function() {
  return Drupal.availabilityCalendar.states;
};

/**
 * @type {Object} Collection of calendar instances.
 */
Drupal.availabilityCalendar.calendars = {};

/**
 *
 * Multiton implementation for accessing calendars on the page.
 *
 * @param {Integer} cid
 * @returns {?Drupal.availabilityCalendar.Calendar}
 */
Drupal.availabilityCalendar.getCalendar = function(cid) {
  return Drupal.availabilityCalendar.calendars[cid] !== undefined ? Drupal.availabilityCalendar.calendars[cid] : null;
};

/**
 * @returns {Object} A list of all calendars on the current page indexed by cid.
 */
Drupal.availabilityCalendar.getCalendars = function() {
  return Drupal.availabilityCalendar.calendars;
};

/**
 * Initialization code that works by implementing the Drupal behaviors paradigm.
 *
 * - The collection of availability states is created.
 * - The calendars that are defined in the settings are created.
 */
Drupal.behaviors.availabilityCalendar = {
    attach: function(context, settings) {
      if (settings.availabilityCalendar.states) {
        if (Drupal.availabilityCalendar.states === null) {
          Drupal.availabilityCalendar.states = new Drupal.availabilityCalendar.States(settings.availabilityCalendar.states);
        }
        else {
          // Reinitialize.
          Drupal.availabilityCalendar.states.init(settings.availabilityCalendar.states);
        }
      }

      if (settings.availabilityCalendar.calendars) {
        var key;
        for (key in settings.availabilityCalendar.calendars) {
          if (settings.availabilityCalendar.calendars.hasOwnProperty(key)) {
            var calendarSettings = settings.availabilityCalendar.calendars[key];
            if (Drupal.availabilityCalendar.calendars[calendarSettings.cid] === undefined) {
              Drupal.availabilityCalendar.calendars[calendarSettings.cid] = new Drupal.availabilityCalendar.Calendar(calendarSettings);
            }
            else {
              Drupal.availabilityCalendar.calendars[calendarSettings.cid].init(calendarSettings);
            }
          }
        }
      }
    }
};

}(jQuery));
