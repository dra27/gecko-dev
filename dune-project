(lang dune 2.7)
(name SpiderMonkey)
(version JS_170)

(generate_opam_files)

(package
  (name SpiderMonkey)
  (synopsis "Firefox SpiderMonkey JavaScript Engine")
  (maintainers "David Allsopp <david@davidallsopp.com>")
  (authors "Mozilla Foundation")
  (license "MPL-2.0")
  (homepage "https://spidermonkey.dev/")
  (bug_reports "https://bugzilla.mozilla.org/home")
  (depends
    conf-g++))
