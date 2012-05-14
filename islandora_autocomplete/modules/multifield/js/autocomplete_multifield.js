
/**
 * Attaches the autocomplete behavior to all required fields
 */
Drupal.behaviors.autocomplete_multifield = function (context) {
  var ACMultiDB = [];
  $('input.autocomplete_multifield:not(.autocomplete-processed)', context).each(function () {
    var id = this.id.substr(0, this.id.length - 13);
    var settings = Drupal.settings.autocomplete_group[id];
    var uri = this.value;
    if (!ACMultiDB[uri + '/' + settings.index]) { // Index's share the same base url.
      ACMultiDB[uri] = new Drupal.ACMultiDB(uri);
    }
    var input = $('#' + id).attr('autocomplete', 'OFF')[0];
    $(input.form).submit(Drupal.autocomplete_multifieldSubmit);
    new Drupal.jsACMulti(input, ACMultiDB[uri]);
    $(this).addClass('autocomplete-processed');
  });
};

/**
 * Prevents the form from submitting if the suggestions popup is open
 * and closes the suggestions popup when doing so.
 */
Drupal.autocomplete_multifieldSubmit = function () {
  return $('#autocomplete').each(function () {
    this.owner.hidePopup();
  }).size() == 0;
};

/**
 * An AutoComplete object
 */
Drupal.jsACMulti = function (input, db) {
  var ac = this;
  this.input = input;
  this.db = db;

  $(this.input)
  .keydown(function (event) {
    return ac.onkeydown(this, event);
  })
  .keyup(function (event) {
    ac.onkeyup(this, event);
  })
  .blur(function () {
    ac.hidePopup();
    ac.db.cancel();
  });

};

/**
 * Handler for the "keydown" event
 */
Drupal.jsACMulti.prototype.onkeydown = function (input, e) {
  if (!e) {
    e = window.event;
  }
  switch (e.keyCode) {
    case 40: // down arrow
      this.selectDown();
      return false;
    case 38: // up arrow
      this.selectUp();
      return false;
    default: // all other keys
      return true;
  }
};

/**
 * Handler for the "keyup" event
 */
Drupal.jsACMulti.prototype.onkeyup = function (input, e) {
  if (!e) {
    e = window.event;
  }
  switch (e.keyCode) {
    case 16: // shift
    case 17: // ctrl
    case 18: // alt
    case 20: // caps lock
    case 33: // page up
    case 34: // page down
    case 35: // end
    case 36: // home
    case 37: // left arrow
    case 38: // up arrow
    case 39: // right arrow
    case 40: // down arrow
      return true;

    case 9:  // tab
    case 13: // enter
    case 27: // esc
      this.hidePopup(e.keyCode);
      return true;

    default: // all other keys
      if (input.value.length > 0)
        this.populatePopup();
      else
        this.hidePopup(e.keyCode);
      return true;
  }
};

/**
 * Puts the currently highlighted suggestion into the autocomplete field
 */
Drupal.jsACMulti.prototype.select = function (node) {
  var inputs = $('#' + node.autocompleteGroup + ' input.form-autocomplete');
  inputs.each(function(index, input) {
    input.value = node.autocompleteValue[index][0];
  });
};

/**
 * Highlights the next suggestion
 */
Drupal.jsACMulti.prototype.selectDown = function () {
  if (this.selected && this.selected.nextSibling) {
    this.highlight(this.selected.nextSibling);
  }
  else {
    var lis = $('li', this.popup);
    if (lis.size() > 0) {
      this.highlight(lis.get(0));
    }
  }
};

/**
 * Highlights the previous suggestion
 */
Drupal.jsACMulti.prototype.selectUp = function () {
  if (this.selected && this.selected.previousSibling) {
    this.highlight(this.selected.previousSibling);
  }
};

/**
 * Highlights a suggestion
 */
Drupal.jsACMulti.prototype.highlight = function (node) {
  if (this.selected) {
    $(this.selected).removeClass('selected');
  }
  $(node).addClass('selected');
  this.selected = node;
};

/**
 * Unhighlights a suggestion
 */
Drupal.jsACMulti.prototype.unhighlight = function (node) {
  $(node).removeClass('selected');
  this.selected = false;
};

/**
 * Hides the autocomplete suggestions
 */
Drupal.jsACMulti.prototype.hidePopup = function (keycode) {
  // Select item if the right key or mousebutton was pressed
  if (this.selected && ((keycode && keycode != 46 && keycode != 8 && keycode != 27) || !keycode)) {
    var selected = this.selected;
    var inputs = $('#' + selected.autocompleteGroup + ' input.form-autocomplete');
    inputs.each(function(index, input) {
      input.value = selected.autocompleteValue[index][0];
    });
  }
  // Hide popup
  var popup = this.popup;
  if (popup) {
    this.popup = null;
    $(popup).fadeOut('fast', function() {
      $(popup).remove();
    });
  }
  this.selected = false;
};

/**
 * Positions the suggestions popup and starts a search
 */
Drupal.jsACMulti.prototype.populatePopup = function () {
  // Show popup
  if (this.popup) {
    $(this.popup).remove();
  }
  this.selected = false;
  this.popup = document.createElement('div');
  this.popup.id = 'autocomplete';
  this.popup.owner = this;
  $(this.popup).css({
    marginTop: this.input.offsetHeight +'px',
    width: (this.input.offsetWidth - 4) +'px',
    display: 'none'
  });
  $(this.input).before(this.popup);

  // Do search
  this.db.owner = this;
  this.db.search(this.input);
};

/**
 * Fills the suggestion popup with any matches received
 */
Drupal.jsACMulti.prototype.found = function (matches) {
  // If no value in the textfield, do not show the popup.
  if (!this.input.value.length) {
    return false;
  }
  // Prepare matches
  var ul = document.createElement('ul');
  var ac = this;
  var settings = Drupal.settings.autocomplete_group[this.input.id];
  for (key in matches) {
    var li = document.createElement('li');
    var display = Drupal.jsACMulti.prototype.formatMatch(settings, matches[key]);
    $(li)
    .html('<div>'+ display +'</div>')
    .mousedown(function () {
      ac.select(this);
    })
    .mouseover(function () {
      ac.highlight(this);
    })
    .mouseout(function () {
      ac.unhighlight(this);
    });
    li.autocompleteGroup = settings.group;
    li.autocompleteValue = matches[key];
    $(ul).append(li);
  }

  // Show popup with matches, if any
  if (this.popup) {
    if (ul.childNodes.length > 0) {
      $(this.popup).empty().append(ul).show();
    }
    else {
      $(this.popup).css({
        visibility: 'hidden'
      });
      this.hidePopup();
    }
  }
};

Drupal.jsACMulti.prototype.formatMatch = function(settings, match) {
  if(settings.format == null) {
    return match[settings.index][1];
  }
  else {
    var output = settings.format;
    match.forEach(function(item, index) {
      var regex = new RegExp("\\\\v" + (index));
      output = output.replace(regex, item[0]);
      regex = new RegExp("\\\\l" + (index));
      output = output.replace(regex, item[1]);
    });
    return output;
  }
  return 'A problem occured...';
}

Drupal.jsACMulti.prototype.setStatus = function (status) {
  switch (status) {
    case 'begin':
      $(this.input).addClass('throbbing');
      break;
    case 'cancel':
    case 'error':
    case 'found':
      $(this.input).removeClass('throbbing');
      break;
  }
};

/**
 * An AutoComplete DataBase object
 */
Drupal.ACMultiDB = function (uri) {
  this.uri = uri;
  this.delay = 300;
  this.cache = {};
};

/**
 * Performs a cached and delayed search
 */
Drupal.ACMultiDB.prototype.search = function (input) {
  var db = this;
  var searchString = input.value;
  this.searchString = searchString;

  // See if this key has been searched for before
  if (this.cache[searchString]) {
    return this.owner.found(this.cache[searchString]);
  }

  // Initiate delayed search
  if (this.timer) {
    clearTimeout(this.timer);
  }
  this.timer = setTimeout(function() {
    db.owner.setStatus('begin');

    // Ajax GET request for autocompletion
    $.ajax({
      type: "GET",
      url: db.uri +'/'+ Drupal.encodeURIComponent(searchString) + '/' + Drupal.settings.autocomplete_group[input.id].index,
      dataType: 'json',
      success: function (matches) {
        if (typeof matches['status'] == 'undefined' || matches['status'] != 0) {
          db.cache[searchString] = matches;
          // Verify if these are still the matches the user wants to see
          if (db.searchString == searchString) {
            db.owner.found(matches);
          }
          db.owner.setStatus('found');
        }
      },
      error: function (xmlhttp) {
        alert(Drupal.ahahError(xmlhttp, db.uri));
      }
    });
  }, this.delay);
};

/**
 * Cancels the current autocomplete request
 */
Drupal.ACMultiDB.prototype.cancel = function() {
  if (this.owner) this.owner.setStatus('cancel');
  if (this.timer) clearTimeout(this.timer);
  this.searchString = '';
};
