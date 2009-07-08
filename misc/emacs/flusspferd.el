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

;;; Code:

(require 'comint)
(require 'easymenu)

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

(defun flusspferd-is-running-p ()
  "Checks if Flusspferd is already running"
  (comint-check-proc flusspferd-buffer-name))

(defun flusspferd-switch-to ()
  "Switch to Flusspferd buffer"
  (interactive)
  (switch-to-buffer flusspferd-buffer-name))

(defmacro flusspferd-defun-and-go (name arglist docstring &rest body)
  "defun a function with name NAME and NAME-and-go which calls
 (switch-to-buffer flusspferd-buffer-name) when finished."
  (let ((name-and-go (intern (concat (symbol-name name) "-and-go")))
        (new-docstring
         (concat docstring " Switches to Flusspferd buffer when finished.")))
    `(progn
       (defun ,name ,arglist ,docstring ,@body)
       (defun ,name-and-go ,arglist ,new-docstring
              ,@body (list flusspferd-switch-to)))))

(flusspferd-defun-and-go flusspferd-eval-region (start end)
  "Evaluates the region between START and END with flusspferd."
  (interactive "r")
  (unless (flusspferd-is-running-p)
    (flusspferd))
  (comint-send-region flusspferd-buffer-name start end)
  (comint-send-string flusspferd-buffer-name "\n"))

(flusspferd-defun-and-go flusspferd-eval-last-defun ()
  "Evaluate the function definition as determined by c-mark-function."
  (interactive)
  (save-excursion
    (c-mark-function)
    (flusspferd-eval-region (point) (mark))))

(flusspferd-defun-and-go flusspferd-eval-last-sexp ()
  "Evaluate last sexp."
  (interactive)
  (flusspferd-eval-region
   (save-excursion (backward-sexp) (point))
   (point)))

(flusspferd-defun-and-go flusspferd-eval-buffer ()
  "Evaluate buffer with Flusspferd."
  (interactive)
  (flusspferd-eval-region (point-min) (point-max)))

(flusspferd-defun-and-go flusspferd-eval-line ()
  "Evaluate current line with Flusspferd."
  (interactive)
  (flusspferd-eval-region
   (save-excursion (move-beginning-of-line nil) (point))
   (save-excursion (move-end-of-line nil) (point))))

(defvar flusspferd-minor-mode-map
  (let ((map (make-sparse-keymap)))
    (define-key map "\C-x\C-e" 'flusspferd-eval-last-sexp)
    (define-key map "\C-c\C-c" 'flusspferd-eval-buffer)
    (define-key map "\C-c\C-r" 'flusspferd-eval-region)
    map))

(defun flusspferd-helper-mark-active ()
  "Checks if there is an active region"
  (if (functionp 'use-region-p)
      (use-region-p)
      (and transient-mark-mode mark-active)))

(easy-menu-define flusspferd-minor-mode-menu flusspferd-minor-mode-map
  "Flusspferd menu"
  '("Flusspferd"
    ["Evaluate Last S-Expression" flusspferd-eval-last-sexp t]
    ["Evaluate Region" flusspferd-eval-region
     :active (flusspferd-helper-mark-active)]
    ["Evaluate Buffer" flusspferd-eval-buffer t]
    ["Evaluate Line" flusspferd-eval-line t :active (not (and (eolp) (bolp)))]
;    ["Evaluate Last Function Definition" flusspferd-eval-last-defun t]
    "--"
    ["Go to Flusspferd buffer" flusspferd-switch-to
     :visible (flusspferd-is-running-p)
     :active (not (string= (buffer-name) flusspferd-buffer-name))]
    ["Start Flusspferd Shell" flusspferd
     :visible (not (flusspferd-is-running-p))]))

;;;###autoload
(define-minor-mode flusspferd-minor-mode
    "Flusspferd minor mode. Provides keymap to evaluate Javascript code with Flusspferd."
  :init-value nil
  :lighter "Flusspferd"
  :keymap flusspferd-minor-mode-map
  (easy-menu-add flusspferd-minor-mode-menu))

(define-derived-mode inferior-flusspferd-mode comint-mode "Inferior Flusspferd"
                     "Embeds Flusspferd as a Javascript REPL into Emacs."
                     :group 'flusspferd
                     (setq comint-prompt-regexp "^> ")
                     (setq comint-use-prompt-regexp t)
                     (setq comint-process-echoes t))

;;;###autoload
(defun flusspferd ()
  "Flusspferd (Javascript interpreter) REPL"
  (interactive)
  (unless (flusspferd-is-running-p)
    (let ((flusspferd-buffer
           (apply 'make-comint-in-buffer "Flusspferd" flusspferd-buffer-name
                  flusspferd-executable nil
                  (split-string flusspferd-options))))
      (save-excursion
        (set-buffer flusspferd-buffer)
		(local-set-key "\C-a" 'comint-bol)
        (inferior-flusspferd-mode))))
  (when (interactive-p)
      (switch-to-buffer flusspferd-buffer-name)))

(provide 'flusspferd)
