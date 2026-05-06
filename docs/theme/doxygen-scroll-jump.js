/*
 @licstart  The following is a small compatibility override for Doxygen output.

 The MIT License (MIT)

 Copyright (C) 2026

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

 @licend
 */

/*
 * Doxygen's navtree.js uses jQuery animate() to smooth-scroll #doc-content when
 * a section anchor is selected. Replace that one path with an immediate jump.
 */
(function ($) {
  "use strict";

  if (!$ || !$.fn || !$.fn.animate) {
    return;
  }

  const originalAnimate = $.fn.animate;

  $.fn.animate = function (properties, duration, easing, complete) {
    let callback = complete;

    if (typeof duration === "function") {
      callback = duration;
    } else if (typeof easing === "function") {
      callback = easing;
    }

    if (
      this.length === 1 &&
      this[0] &&
      this[0].id === "doc-content" &&
      properties &&
      typeof properties === "object" &&
      Object.keys(properties).length === 1 &&
      Object.prototype.hasOwnProperty.call(properties, "scrollTop")
    ) {
      this.scrollTop(properties.scrollTop);
      if (typeof callback === "function") {
        callback.call(this[0]);
      }
      return this;
    }

    return originalAnimate.apply(this, arguments);
  };
})(window.jQuery);
