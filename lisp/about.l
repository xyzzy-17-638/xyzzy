;;; -*- Mode: Lisp; Package: EDITOR -*-
;;;
;;; This file is not part of xyzzy.
;;;

(require "ni/setup")
(netinst::load-install-data)

(provide "niseaboutd")

(in-package "editor")

(let ((dialog-template nil))
  (defun about-dialog ()
    (interactive)
    (unless dialog-template
      (setq dialog-template
	    `(dialog 0 0 211 220
		     (:caption "‚É‚Â‚¢‚Ä")
		     (:font 9 "MS UI Gothic")
		     (:control
		      (:button IDOK "‚¨‚¯" #x50010001 171 7 33 14)
		      (:static nil nil #x50020003 7 7 18 20)
		      (:link xyzzy ,(concat "\001xyzzy\002 version "
					    (software-version-display-string))
		       #x50020000 34 7 123 8)
		      (:link mail ,(format-date-string "Copyright (C) 1996-%Y \001Tetsuya Kamei\002" (get-universal-time))
		       #x50020000 34 19 132 8)
		      (:static nil "All rights reserved." #x50020000 34 28 68 8)
		      (:static nil "Icon designed by Yoshinori Watanabe."
		       #x50020000 34 40 126 8)
		      (:static nil "RSA Data Security, Inc." #x50020000 34 52 83 8)
		      (:static nil "MD5 Message-Digest Algorithm." #x50020000 34 61 111 8)
		      (:listbox list  nil #x50a10111 33 72 138 60)
		      (:static nil "Šg’£lisp" #x50020000 34 140 111 8)
		      (:listbox lis   nil #x50a10111 33 150 138 60)))))
    (dialog-box dialog-template
		(list (cons 'list
			    (long-operation
			      (mapcan #'(lambda (x)
					  (let ((version (archiver-dll-version x)))
					    (and version
						 (list (list (get x 'archiver-dll-file-name)
							     version)))))
				      *archiver-dll-list*)))
		      (when (find "ni/setup" *modules* :test 'string=)
			(cons 'lis
			      (mapcan #'(lambda (x)
					  (list
					   (list (cdr (assoc "name" x :test 'string=))
						 (cdr (assoc "version" x :test 'string=)))))
				      (sort (copy-list netinst::*install-data*)
					    #'string-lessp
					    :key #'(lambda (x) (cdr (assoc "name" x :test 'string=))))))))
		'((xyzzy :url "http://www.jsdlab.co.jp/~kamei/")
		  (mail :url "mailto:kamei@jsdlab.co.jp")
		  (list :column (28 10))
		  (lis :column (28 10))))))
