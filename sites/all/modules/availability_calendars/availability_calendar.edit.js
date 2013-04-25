(function($) {
"use strict";

/**
 * @class Drupal.availabilityCalendar.Command represents a calendar state
 * changing command during the whole creation phase, i.e. from click on a state
 * to the click on the end date.
 *
 * @param {Drupal.availabilityCalendar.Calendar} calendar
 * @param {*} fieldContext
 * @constructor
 */
Drupal.availabilityCalendar.Command = function(calendar, fieldContext) {
  this.state = "";
  this.from = null;
  this.to = null;
  this.elt = $(".availability-changes", fieldContext);

  /**
   * Sets the state of the current command and, as this is supposed to be the
   * first parameter to be set, cleans the from and to dates.
   *
   * @param {String} [selectedState]
   */
  this.setState = function(selectedState) {
    if (selectedState !== undefined) {
      this.state = selectedState;
    }
    this.from = null;
    this.to = null;
    this.show();
  };

  /**
   * Adds a date to the command. If it is the 1st date it will be the from date.
   * If it is the 2nd date it will be the to date, if necessary, swapping the
   * from and to dates.
   *
   * @param {Date} date
   */
  this.addDate = function(date) {
    if (this.from === null) {
      this.from = date;
    }
    else if (this.to === null) {
      this.to = date;
      if (this.to < this.from) {
        var from = this.from;
        this.from = this.to;
        this.to = from;
      }
      if (calendar.isOvernight()) {
        // Overnight rental: to date = departure date = am only: store as "from"
        // to "to - 1 day". But in the case of clicking twice on the same day,
        // it will be handled as a 1 night booking.
        if (this.to > this.from) {
          this.to.setDate(this.to.getDate() - 1);
        }
      }
    }
    this.show();
  };

  /**
   * @returns {Boolean} Whether the current command is complete.
   */
  this.isComplete = function() {
    return this.to !== null && this.from !== null && this.state !== '';
  };

  /**
   * Replaces the current command in the accompanying hidden field.
   */
  this.show = function() {
    var val = this.elt.val();
    var pos = val.lastIndexOf("\n") + 1;
    val = val.substr(0, pos) + this.toString();
    this.elt.val(val);
  };

  /**
   * Finishes the current command and starts a new one.
   */
  this.finish = function() {
    this.show();
    this.elt.val(this.elt.val() + "\n");
    this.setState();
  };

  /**
   * @returns String
   *   A string representation of the current command.
   */
  this.toString = function() {
    var result = "";
    result += "state: ";
    result += this.state !== "" ? this.state : "-";
    result += " from: ";
    result += this.from !== null ? Drupal.availabilityCalendar.formatIsoDate(this.from) : "-";
    result += " to: ";
    result += this.to !== null ? Drupal.availabilityCalendar.formatIsoDate(this.to) : "-";
    return result;
  };
};

/**
 * @class Drupal.availabilityCalendar.Editor
 *   Provides the glueing code that connects the form elements on entity edit
 *   forms (for entities with an availability calendar field) with the
 *   Drupal.availabilityCalendar.Calendar API object and the
 *   Drupal.availabilityCalendar.Command class.
 *
 * @param {Object} settings
 * @param {Integer} settings.cvid
 */
Drupal.availabilityCalendar.Editor = function(settings) {
  /** @type {Drupal.availabilityCalendar.Calendar} */
  var calendar;
  var view;
  var editorSettings;
  var fieldContext;
  var formRadios;
  var command;
  var calSelectedDay;

  this.init = function(settings) {
    editorSettings = settings;
    view = Drupal.availabilityCalendar.getCalendarView(editorSettings.cvid);
    calendar = view.getCalendar();
    fieldContext = view.getView().parents('.form-wrapper').first();
    fieldContext.once('Drupal-availabilityCalendar-Editor', function () {
      formRadios = $(".form-radios.availability-states", fieldContext);
      initCommand();
      attach2Checkbox();
      attach2States();
      bind2CalendarEvents();
    });
  };

  /**
   * Initialize a new Drupal.availabilityCalendar.Command object.
   */
  function initCommand() {
    command = new Drupal.availabilityCalendar.Command(calendar, fieldContext);
    command.setState($(":radio:checked", formRadios).val());
    // Add css_class of states as class to wrapper elements around the radios.
    $(":radio", formRadios).parent().addClass(function() {
      return Drupal.availabilityCalendar.getStates().get(($(this).children(":radio:").val())).cssClass;
    });
  }

  /**
   * Attach to "enable calendar" checkbox (if it exists).
   */
  function attach2Checkbox() {
    var enable = $('.availability-enable', fieldContext);
    if (enable.size() > 0 ) {
      $('.availability-details', fieldContext).toggle(enable.filter(':checked').size() > 0);
      enable.click(function () {
        $('.availability-details', fieldContext).toggle('fast');
      });
    }
  }

  /**
   * Attach to state radios events.
   */
  function attach2States() {
    $("input:radio", formRadios).click(function() {
      // State clicked: remove cal-selected and restart current command.
      removeCalSelected();
      command.setState($(":radio:checked", formRadios).val());
    });
  }

  /**
   * Attach to the calendar calendarClick event.
   */
  function bind2CalendarEvents() {
    $(calendar).bind("calendarClick", function(event, date/*, cid, view*/) {
      command.addDate(date);
      if (!command.isComplete()) {
        setCalSelected(command.from);
      }
      else {
        removeCalSelected();
        calendar.changeRangeState(command.from, command.to, command.state);
        command.finish();
      }
    });
  }

  /**
   * Set the cal-selected class on the day cell of the given date.
   *
   * @param {Date} day
   */
  function setCalSelected(day) {
    removeCalSelected();
    calSelectedDay = new Date(day.getTime());
    calendar.addExtraState(calSelectedDay, "cal-selected");
  }

  /**
   * Removes the cal-selected class on the day cell where it was last set on.
   */
  function removeCalSelected() {
    if (calSelectedDay !== null) {
      calendar.removeExtraState(calSelectedDay, "cal-selected");
      calSelectedDay = null;
    }
  }

  this.init(settings);
};

/**
 * @type {Object} Collection of calendar editor instances.
 */
Drupal.availabilityCalendar.editors = {};

/**
 * Multiton implementation for accessing calendar editors on the page.
 *
 * @param {Integer} cvid
 * @return {?Drupal.availabilityCalendar.Editor}
 */
Drupal.availabilityCalendar.getEditor = function(cvid) {
  return Drupal.availabilityCalendar.editors[cvid] !== undefined ? Drupal.availabilityCalendar.editors[cvid] : null;
};

/**
 * Initialization code that works by implementing the Drupal behaviors paradigm.
 *
 * Based on the information in the settings, the calendar editors are created.
 */
Drupal.behaviors.availabilityCalendarEditor = {
  attach: function(context, settings) {
    if (settings.availabilityCalendar.editors) {
      var key;
      for (key in settings.availabilityCalendar.editors) {
        if (settings.availabilityCalendar.editors.hasOwnProperty(key)) {
          var editorSettings = settings.availabilityCalendar.editors[key];
          if (Drupal.availabilityCalendar.editors[editorSettings.cvid] === undefined) {
            Drupal.availabilityCalendar.editors[editorSettings.cvid] = new Drupal.availabilityCalendar.Editor(editorSettings);
          }
          else {
            Drupal.availabilityCalendar.editors[editorSettings.cvid].init(editorSettings);
          }
        }
      }
    }
  }
};

}(jQuery));
