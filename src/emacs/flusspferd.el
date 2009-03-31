;;; flusspferd.el --- Interactive interface to the Flusspferd REPL
;; Copyright (C) 2009
;;   Rüdiger Sonderfeld, Aristid Breitkreuz
;;
;; Author: Rüdiger Sonderfeld <ruediger at c-plusplus dot de>
;; Homepage: http://flusspferd.org
;;
;; This file is NOT part of GNU Emacs.
;;
;; Permission is hereby granted, free of charge, to any person obtaining a copy
;; of this software and associated documentation files (the "Software"), to deal
;; in the Software without restriction, including without limitation the rights
;; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
;; copies of the Software, and to permit persons to whom the Software is
;; furnished to do so, subject to the following conditions:

;; The above copyright notice and this permission notice shall be included in
;; all copies or substantial portions of the Software.

;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
;; THE SOFTWARE.

;;; Commentary:
;;
;; ...

(require 'comint)

;; Options
(defgroup flusspferd nil
  "Flusspferd (Javascript interpreter) REPL"
  :version "0.5pre"
  :link '(url-link "http://flusspferd.org")
  :prefix "flusspferd-"
  :group 'external)

(defcustom flusspferd-executable "flusspferd"
  "Flusspferd executable."
  :group 'flusspferd
  :type '(string))

(defcustom flusspferd-options ""
  "Options for the Flusspferd executable."
  :group 'flusspferd
  :type '(string))

(defcustom flusspferd-buffer-name "*Flusspferd*"
  "Name of the Flusspferd buffer."
  :group 'flusspferd
  :type '(string))

(defun flusspferd-mode () )

;;;###autoload
(defun flusspferd ()
  "Flusspferd (Javascript interpreter) REPL"
  (interactive)
  (unless (comint-check-proc flusspferd-buffer-name)
    (let ((flusspferd-buffer
           (apply 'make-comint-in-buffer "Flusspferd" flusspferd-buffer-name
                  flusspferd-executable nil
                  (split-string flusspferd-options))))
      (save-excursion
        (set-buffer flusspferd-buffer)
        (flusspferd-mode))))
  (when (interactive-p)
      (switch-to-buffer flusspferd-buffer-name)))

;; TODO flusspferd-eval-region flusspferd-eval-last-sexp flusspferd-eval-buffer flusspferd-minor-mode and so on

(provide 'flusspferd)
