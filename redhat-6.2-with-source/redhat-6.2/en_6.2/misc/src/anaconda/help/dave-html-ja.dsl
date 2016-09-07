<!DOCTYPE style-sheet PUBLIC "-//James Clark//DTD DSSSL Style Sheet//EN" [
<!ENTITY dbstyle SYSTEM "/usr/lib/sgml/stylesheets/nwalsh-modular/html/docbook.dsl" CDATA DSSSL>
]>
<style-sheet>
<style-specification id="html" use="docbook">
<style-specification-body>



;;#######################################################################
;;#                                                                     #
;;#    Dave's Custom DocBook Stylesheet by Dave Mason dcm@redhat.com    #
;;#            Based on Norman Walsh's Modular Stylesheets              #
;;#                                                                     #
;;#######################################################################

(declare-characteristic preserve-sdata?
          "UNREGISTERED::James Clark//Characteristic::preserve-sdata?"
          #f)

;;=========================
;;General Stuff
;;=========================

;;Should there be a link to the legalnotice?
(define %generate-legalnotice-link%
  #t)

;;Should Admon Graphics be used?
(define %admon-graphics%
  #f)

;;Where are those admon graphics?
(define %admon-graphics-path%
  "./stylesheet-images/")

;;What graphics extensions allowed?
(define %graphic-extensions% 
'("gif" "jpg" "jpeg" "tif" "tiff" "eps" "epsf" ))

;;What is the default extension for images?
(define %graphic-default-extension% "gif")

;;Use element ids as filenames?
(define %use-id-as-filename%
 #t)

;;=========================
;;Book Stuff
;;=========================

;;Do you want a TOC for Books?
(define %generate-book-toc% 
  #f)

;;What elements should have an LOT?
(define ($generate-book-lot-list$)
  (list (normalize "table")
  (normalize "example")
  (normalize "equation")))

;;Do you want a title page for your Book?
(define %generate-book-titlepage%
#f)

;;=========================
;;Part Stuff
;;=========================

;;Should parts have TOCs?
(define %generate-part-toc% 
  #f)

;;Should part TOCs be on their titlepages?
(define %generate-part-toc-on-titlepage%
  #f)

;;Do you want a title page for your part?
(define %generate-part-titlepage% 
  #f)

;;Should the Part intro be on the part title page?
(define %generate-partintro-on-titlepage%
 #t)

;;=========================
;;Article Stuff
;;=========================

;;Should Articles have a TOC?
(define %generate-article-toc% 
  #f)

;;========================
;;Chapter Stuff
;;=======================

(define $generate-chapter-toc$
 (lambda ()
    #f))

;;=========================
;;Navigation
;;=========================

;;Should there be navigation at top?
(define %header-navigation%
 #f)

;;Should there be navigation at bottom?
(define %footer-navigation%
  #f)

;;Use tables to create the navigation?
(define %gentext-nav-use-tables%
 #f)

;;If tables are used for navigation, how wide should they be? 
(define %gentext-nav-tblwidth% 
"100%")

;;=========================
;;Tables and Lists
;;=========================

;;Should Variable lists be tables?
(define %always-format-variablelist-as-table%
 #f)

;;What is the length of the 'Term' in a variablelist?
(define %default-variablelist-termlength%
  20)

;;When true.. If the terms are shorter than the termlength above then
;;the variablelist will be formatted as a table.
(define %may-format-variablelist-as-table%
#f)

;; This overrides the tgroup definition (copied from 1.20, dbtable.dsl).
;; It changes the table background color, cell spacing and cell padding.
(element tgroup
  (let* ((wrapper   (parent (current-node)))
	 (frameattr (attribute-string (normalize "frame") wrapper))
	 (pgwide    (attribute-string (normalize "pgwide") wrapper))
	 (footnotes (select-elements (descendants (current-node)) 
				     (normalize "footnote")))
	 (border (if (equal? frameattr (normalize "none"))
		     '(("BORDER" "0"))
		     '(("BORDER" "1"))))
	 (bgcolor '(("BGCOLOR" "#E0E0E0")))
	 (width (if (equal? pgwide "1")
		    (list (list "WIDTH" ($table-width$)))
		    '()))
	 (head (select-elements (children (current-node)) (normalize "thead")))
	 (body (select-elements (children (current-node)) (normalize "tbody")))
	 (feet (select-elements (children (current-node)) (normalize "tfoot"))))
    (make element gi: "TABLE"
	  attributes: (append
		       border
		       width
		       bgcolor
		       '(("CELLSPACING" "0"))
		       '(("CELLPADDING" "4"))
		       (if %cals-table-class%
			   (list (list "CLASS" %cals-table-class%))
			   '()))
	  (process-node-list head)
	  (process-node-list body)
	  (process-node-list feet)
	  (make-table-endnotes))))

;;=========================
;;Labels
;;=========================

;;Enumerate Chapters?
(define %chapter-autolabel% 
 #f)

;;Enumerate Sections?
(define %section-autolabel%
 #f)

;;=========================
;;HTML Attributes
;;=========================

;;What attributes should be hung off of 'body'?
(define %body-attr%
 (list
   (list "BGCOLOR" "#FFFFFF")
   (list "TEXT" "#000000")
   (list "LINK" "#0000FF")
   (list "VLINK" "#840084")
   (list "ALINK" "#0000FF")))

;;Default extension for filenames?
(define %html-ext% 
  ".html")

;;=========================
;;Elements
;;=========================

;;Indent addresses?
(define %indent-address-lines%
  #f)

;;Indent Literal layouts?
(define %indent-literallayout-lines% 
  #f)

;;Indent Programlistings?
(define %indent-programlisting-lines%
  #f)

;;Number lines in Programlistings?
(define %number-programlisting-lines%
 #f)

;;Should verbatim items be 'shaded' with a table?
(define %shade-verbatim% 
 #t)

;;Define shade-verbatim attributes
(define ($shade-verbatim-attr$)
 (list
  (list "BORDER" "0")
  (list "BGCOLOR" "#E0E0E0")
  (list "WIDTH" ($table-width$))))

;;=================INLINES====================

(element application ($mono-seq$))
(element command ($bold-seq$))
(element filename ($mono-seq$))
(element function ($mono-seq$))
(element guibutton ($bold-seq$))
(element guiicon ($bold-seq$))
(element guilabel ($bold-seq$))
(element guimenu ($bold-seq$))
(element guimenuitem ($bold-seq$))
(element hardware ($bold-mono-seq$))
(element keycap ($bold-seq$))
(element literal ($mono-seq$))
(element parameter ($bold-mono-seq$))
(element prompt ($mono-seq$))
(element symbol ($charseq$))
(element emphasis ($bold-seq$))

;=============================================



;;=========================
;;Title Pages For Articles
;;=========================

(define (article-titlepage-recto-elements)
  (list (normalize "title")
	(normalize "subtitle")
	(normalize "corpauthor")
	(normalize "authorgroup")
	(normalize "author")
	(normalize "releaseinfo")
	(normalize "copyright")
	(normalize "pubdate")
	(normalize "revhistory")
	(normalize "abstract")))


(element abstract
    (make element gi: "DIV"
	  ($semiformal-object$)))

(element address 
    (make element gi: "DIV"
	  attributes: (list (list "CLASS" (gi)))
	  (with-mode titlepage-address-mode 
	    ($linespecific-display$ %indent-address-lines% %number-address-lines%))))


 (element affiliation
    (make element gi: "DIV"
	  attributes: (list (list "CLASS" (gi)))
	  (process-children)))


(element author
    (let ((author-name  (author-string))
	  (author-affil (select-elements (children (current-node)) 
					 (normalize "affiliation"))))
      (make sequence      
	(make element gi: "H3"
	      attributes: (list (list "CLASS" (gi)))
              (literal "by ")
	      (literal author-name))
	(process-node-list author-affil))))


;;============================
;;Title for Formal Paragraphs
;;============================

(element formalpara
  (make element gi: "DIV"
	attributes: (list
		     (list "CLASS" (gi)))
  	(make element gi: "P"
	      (process-children))))

;(element (formalpara title) ($lowtitle$ 5))
(element (formalpara title) 
  (make element gi: "B"
	($runinhead$)))



;;========================================
;;Change the Nav, derived from gtk-doc.dsl
;;========================================


;; This overrides default-header-nav-tbl-noff (copied from 1.20, dbnavig.dsl).
;; I want 'Home' and 'Up' links at the top of each page, and white text on
;; black.
(define (default-header-nav-tbl-noff elemnode prev next prevsib nextsib)
  (let* ((r1? (nav-banner? elemnode))
	 (r1-sosofo (make element gi: "TR"
			  (make element gi: "TH"
				attributes: (list
					     (list "COLSPAN" "4")
					     (list "ALIGN" "center"))
				(make element gi: "FONT"
				      attributes: (list
						   (list "COLOR" "#000000")
						   (list "SIZE" "2"))
				      (nav-banner elemnode)))))
	 (r2? (or (not (node-list-empty? prev))
		  (not (node-list-empty? next))
		  (nav-context? elemnode)))
	 (r2-sosofo (make element gi: "TR"
			  (make element gi: "TD"
				attributes: (list
					     (list "WIDTH" "25%")
					     (list "BGCOLOR" "#ffffff")
					     (list "ALIGN" "left"))
				(if (node-list-empty? prev)
				    (make entity-ref name: "nbsp")
				    (make element gi: "A"
					  attributes: (list
						       (list "HREF" 
							     (href-to 
							      prev)))
					  (make element gi: "FONT"
						attributes: (list
							     (list "COLOR" "#0000ff")
							     (list "SIZE" "2"))
						(make element gi: "B"
						      (gentext-nav-prev prev))))))
			 (make element gi: "TD"
			       attributes: (list
					    (list "WIDTH" "25%")
					    (list "COLSPAN" "2")
					    (list "BGCOLOR" "#ffffff")
					    (list "ALIGN" "center"))
					  (make element gi: "FONT"
						attributes: (list
							     (list "COLOR" "#0000ff")
							     (list "SIZE" "2"))
						(make element gi: "B"
						      (nav-home-link elemnode))))
;Not sure why this is here?
;			 (make element gi: "TD"
;			       attributes: (list
;					    (list "WIDTH" "25%")
;					    (list "BGCOLOR" "#ffffff")
;					    (list "ALIGN" "center"))
;					  (make element gi: "FONT"
;						attributes: (list
;							     (list "COLOR" "#000000")
;							     (list "SIZE" "2"))
;						(make element gi: "B"
;						      (if (nav-up? elemnode)
;							  (nav-up elemnode)
;							  (make entity-ref name: "nbsp")))))
			  (make element gi: "TD"
				attributes: (list
					     (list "WIDTH" "25%")
					     (list "BGCOLOR" "#ffffff")
					     (list "ALIGN" "right"))
				(if (node-list-empty? next)
				    (make entity-ref name: "nbsp")
				    (make element gi: "A"
					  attributes: (list
						       (list "HREF" 
							     (href-to
							      next)))
					  (make element gi: "FONT"
						attributes: (list
							     (list "COLOR" "#0000ff")
							     (list "SIZE" "2"))
						(make element gi: "B"
						      (gentext-nav-next next)))))))))
    (if (or r1? r2?)
	(make element gi: "DIV"
	      attributes: '(("CLASS" "NAVHEADER"))
	  (make element gi: "TABLE"
		attributes: (list
			     (list "WIDTH" %gentext-nav-tblwidth%)
			     (list "BORDER" "0")
			     (list "BGCOLOR" "#ffffff")
			     (list "CELLPADDING" "1")
			     (list "CELLSPACING" "0"))
		(if r1? r1-sosofo (empty-sosofo))
		(if r2? r2-sosofo (empty-sosofo))))
	(empty-sosofo))))


;; This overrides default-footer-nav-tbl (copied from 1.20, dbnavig.dsl).
;; It matches the header above.
(define (default-footer-nav-tbl elemnode prev next prevsib nextsib)
  (let ((r1? (or (not (node-list-empty? prev))
		 (not (node-list-empty? next))
		 (nav-home? elemnode)))
	(r2? (or (not (node-list-empty? prev))
		 (not (node-list-empty? next))
		 (nav-up? elemnode)))

	(r1-sosofo (make element gi: "TR"
			  (make element gi: "TD"
				attributes: (list
					     (list "WIDTH" "25%")
					     (list "BGCOLOR" "#ffffff")
					     (list "ALIGN" "left"))
				(if (node-list-empty? prev)
				    (make entity-ref name: "nbsp")
				    (make element gi: "A"
					  attributes: (list
						       (list "HREF" 
							     (href-to 
							      prev)))
					  (make element gi: "FONT"
						attributes: (list
							     (list "COLOR" "#0000ff")
							     (list "SIZE" "2"))
						(make element gi: "B"
						      (gentext-nav-prev prev))))))
			 (make element gi: "TD"
			       attributes: (list
					    (list "WIDTH" "25%")
					    (list "COLSPAN" "2")
					    (list "BGCOLOR" "#ffffff")
					    (list "ALIGN" "center"))
					  (make element gi: "FONT"
						attributes: (list
							     (list "COLOR" "#0000ff")
							     (list "SIZE" "2"))
						(make element gi: "B"
						      (nav-home-link elemnode))))
;;Not sure why this is here?
;			 (make element gi: "TD"
;			       attributes: (list
;					    (list "WIDTH" "25%")
;					    (list "BGCOLOR" "#ffffff")
;					    (list "ALIGN" "center"))
;					  (make element gi: "FONT"
;						attributes: (list
;							     (list "COLOR" "#000000")
;							     (list "SIZE" "2"))
;						(make element gi: "B"
;						      (if (nav-up? elemnode)
;							  (nav-up elemnode)
;							  (make entity-ref name: "nbsp")))))
			  (make element gi: "TD"
				attributes: (list
					     (list "WIDTH" "25%")
					     (list "BGCOLOR" "#ffffff")
					     (list "ALIGN" "right"))
				(if (node-list-empty? next)
				    (make entity-ref name: "nbsp")
				    (make element gi: "A"
					  attributes: (list
						       (list "HREF" 
							     (href-to
							      next)))
					  (make element gi: "FONT"
						attributes: (list
							     (list "COLOR" "#0000ff")
							     (list "SIZE" "2"))
						(make element gi: "B"
						      (gentext-nav-next next))))))))

	(r2-sosofo (make element gi: "TR"
			 (make element gi: "TD"
			       attributes: (list
					    (list "COLSPAN" "2")
					    (list "ALIGN" "left"))
			       (if (node-list-empty? prev)
				   (make entity-ref name: "nbsp")
				   (make element gi: "FONT"
					 attributes: (list
						      (list "COLOR" "#000000")
						      (list "SIZE" "2"))
					 (make element gi: "B"
					       (element-title-sosofo prev)))))
			 (make element gi: "TD"
			       attributes: (list
					    (list "COLSPAN" "2")
					    (list "ALIGN" "right"))
			       (if (node-list-empty? next)
				   (make entity-ref name: "nbsp")
				   (make element gi: "FONT"
					 attributes: (list
						      (list "COLOR" "#000000")
						      (list "SIZE" "2"))
					 (make element gi: "B"
					       (element-title-sosofo next))))))))
    (if (or r1? r2?)
	(make element gi: "DIV"
	      attributes: '(("CLASS" "NAVFOOTER"))
	      (make empty-element gi: "BR")
	      (make empty-element gi: "BR")

	      (make element gi: "TABLE"
		    attributes: (list
				 (list "WIDTH" %gentext-nav-tblwidth%)
				 (list "BORDER" "0")
				 (list "BGCOLOR" "#ffffff")
				 (list "CELLPADDING" "1")
				 (list "CELLSPACING" "0"))
		    (if r1? r1-sosofo (empty-sosofo))
		    (if r2? r2-sosofo (empty-sosofo))))
	(empty-sosofo))))


;; This overrides nav-up (copied from 1.20, dbnavig.dsl).
;; We want to change the size and color of the text.

(define (nav-up elemnode)
  (let ((up (parent elemnode)))
    (if (or (node-list-empty? up)
	    (node-list=? up (sgml-root-element)))
	(make entity-ref name: "nbsp")
	(make element gi: "A"
	      attributes: (list
			   (list "HREF" (href-to up)))
	      (make element gi: "FONT"
		    attributes: (list
				 (list "COLOR" "#0000ff")
				 (list "SIZE" "2"))
		    (make element gi: "B"
			  (gentext-nav-up up)))))))

;; This overrides nav-home-link (copied from 1.20, dbnavig.dsl).
;; We want to change the size and color of the text.

(define (nav-home-link elemnode)
  (let ((home (nav-home elemnode)))
    (if (node-list=? elemnode home)
	(make entity-ref name: "nbsp")
	(make element gi: "A"
	      attributes: (list
			   (list "HREF" 
				 (href-to home)))
	      (make element gi: "FONT"
		    attributes: (list
				 (list "COLOR" "#0000ff")
				 (list "SIZE" "2"))
		    (make element gi: "B"
			  (gentext-nav-home home)))))))

;; These override 2 functions which return the English text to use for links to
;; previous and next pages. (copied from 1.20, dbl1en.dsl).

(define (gentext-en-nav-prev prev) 
  (make sequence (literal "<<< Previous")))

(define (gentext-en-nav-next next)
  (make sequence (literal "Next >>>")))

(define ($object-titles-after$)
  (list (normalize "figure")))

</style-specification-body>
</style-specification>
<external-specification id="docbook" document="dbstyle">
</style-sheet>




