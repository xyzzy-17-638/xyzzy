;;; -*- Mode: Lisp; Package: EDITOR -*-
;;;
;;; This file is part of xyzzy.
;;;

(provide "frame")

(in-package "editor")

(export '(delete-other-frames *frame-init-width* *frame-init-height*))

(defun delete-other-frames() 
  (interactive)
  (let ((cur (selected-frame)))
    (dolist (frame (frame-list))
      (unless (eq cur frame)
	(delete-frame frame)))))

;; this is called ealier than keymap.l.
;; I can remove this from keymap.l.
;; But I don't want to rely those fragile order, so I just keep both. 
(defvar ctl-x-5-map (make-sparse-keymap))
(define-key ctl-x-5-map #\2 'make-frame)
(define-key ctl-x-5-map #\o 'other-frame)
(define-key ctl-x-5-map #\1 'delete-other-frames)
(define-key ctl-x-5-map #\0 'delete-frame)

(defvar *frame-init-width* nil)
(defvar *frame-init-height* nil)

