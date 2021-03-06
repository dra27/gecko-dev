;;; The contents of this file are subject to the Mozilla Public
;;; License Version 1.1 (the "License"); you may not use this file
;;; except in compliance with the License. You may obtain a copy of
;;; the License at http://www.mozilla.org/MPL/
;;; 
;;; Software distributed under the License is distributed on an "AS
;;; IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
;;; implied. See the License for the specific language governing
;;; rights and limitations under the License.
;;; 
;;; The Original Code is the Language Design and Prototyping Environment.
;;; 
;;; The Initial Developer of the Original Code is Netscape Communications
;;; Corporation.  Portions created by Netscape Communications Corporation are
;;; Copyright (C) 1999-2002 Netscape Communications Corporation.  All
;;; Rights Reserved.
;;; 
;;; Contributor(s):   Waldemar Horwat <waldemar@acm.org>
;;;
;;; Alternatively, the contents of this file may be used under the terms of
;;; either the GNU General Public License Version 2 or later (the "GPL"), or
;;; the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
;;; in which case the provisions of the GPL or the LGPL are applicable instead
;;; of those above. If you wish to allow use of your version of this file only
;;; under the terms of either the GPL or the LGPL, and not to allow others to
;;; use your version of this file under the terms of the MPL, indicate your
;;; decision by deleting the provisions above and replace them with the notice
;;; and other provisions required by the GPL or the LGPL. If you do not delete
;;; the provisions above, a recipient may use your version of this file under
;;; the terms of any one of the MPL, the GPL or the LGPL.

;;;
;;; Line-break sensitive test grammar
;;;
;;; Waldemar Horwat (waldemar@acm.org)
;;;

(declaim (optimize (debug 3)))

(progn
  (defparameter *ltw*
    (generate-world
     "T"
     '((line-grammar line-test-grammar :lalr-1 :start)
       
       (production :start (a) start-a)
       (production :start (b :no-line-break c) start-b-c)
       (production :start (d :no-line-break :y z) start-d-y-z)
       (production :start (e :y z) start-e-y-z)
       (production :start (:q :no-line-break a) start-q-a)
       (production :start (c :q a) start-c-q-a)
       (production :y () y-empty)
       (production :y (x) y-x)
       (production :q (x x) q-x-x)
       )))
  
  (defparameter *ltg* (world-grammar *ltw* 'line-test-grammar)))


#|
(depict-rtf-to-local-file
 "Test/LineTestGrammar.rtf"
 "Line Test Grammar"
 #'(lambda (markup-stream)
     (depict-world-commands markup-stream *ltw* :heading-offset 1 :visible-semantics nil)))

(depict-html-to-local-file
 "Test/LineTestGrammar.html"
 "Line Test Grammar"
 t
 #'(lambda (markup-stream)
     (depict-world-commands markup-stream *ltw* :heading-offset 1 :visible-semantics nil)))

(print-grammar *ltg*)
(with-local-output (s "Test/LineTestGrammar.txt") (print-grammar *ltg* s))

;(pprint (parse *ltg* #'identity '(begin letter letter letter digit end)))
|#

(length (grammar-states *ltg*))
