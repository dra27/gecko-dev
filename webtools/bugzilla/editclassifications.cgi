#!/usr/bin/perl -wT
# -*- Mode: perl; indent-tabs-mode: nil; cperl-indent-level: 4 -*-
#
# The contents of this file are subject to the Mozilla Public
# License Version 1.1 (the "License"); you may not use this file
# except in compliance with the License. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS
# IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
# implied. See the License for the specific language governing
# rights and limitations under the License.
#
# The Original Code is the Bugzilla Bug Tracking System.
#
# The Initial Developer of the Original Code is Albert Ting
#
# Contributor(s): Albert Ting <alt@sonic.net>
#                 Max Kanat-Alexander <mkanat@bugzilla.org>
#
# Direct any questions on this source code to mozilla.org

use strict;
use lib ".";

use Bugzilla;
use Bugzilla::Constants;
use Bugzilla::Util;
use Bugzilla::Error;
use Bugzilla::Config qw($datadir);
use Bugzilla::Classification;
use Bugzilla::Product;

require "globals.pl";

my $cgi = Bugzilla->cgi;
my $dbh = Bugzilla->dbh;
my $template = Bugzilla->template;
my $vars = {};

sub LoadTemplate ($) {
    my $action = shift;

    $action =~ /(\w+)/;
    $action = $1;
    print $cgi->header();
    $template->process("admin/classifications/$action.html.tmpl", $vars)
      || ThrowTemplateError($template->error());
    exit;
}

#
# Preliminary checks:
#

Bugzilla->login(LOGIN_REQUIRED);

print $cgi->header();

exists Bugzilla->user->groups->{'editclassifications'}
  || ThrowUserError("auth_failure", {group  => "editclassifications",
                                     action => "edit",
                                     object => "classifications"});

ThrowUserError("auth_classification_not_enabled") unless Param("useclassification");

#
# often used variables
#
my $action     = trim($cgi->param('action')         || '');
my $class_name = trim($cgi->param('classification') || '');
    
#
# action='' -> Show nice list of classifications
#

unless ($action) {
    my @classifications =
        Bugzilla::Classification::get_all_classifications();

    $vars->{'classifications'} = \@classifications;
    LoadTemplate("select");
}

#
# action='add' -> present form for parameters for new classification
#
# (next action will be 'new')
#

if ($action eq 'add') {
    LoadTemplate($action);
}

#
# action='new' -> add classification entered in the 'action=add' screen
#

if ($action eq 'new') {

    $class_name || ThrowUserError("classification_not_specified");

    my $classification =
        new Bugzilla::Classification({name => $class_name});

    if ($classification) {
        ThrowUserError("classification_already_exists",
                       { name => $classification->name });
    }
    
    my $description = trim($cgi->param('description')  || '');
    trick_taint($description);
    trick_taint($class_name);

    # Add the new classification.
    $dbh->do("INSERT INTO classifications (name, description)
              VALUES (?, ?)", undef, ($class_name, $description));

    # Make versioncache flush
    unlink "$datadir/versioncache";

    LoadTemplate($action);
}

#
# action='del' -> ask if user really wants to delete
#
# (next action would be 'delete')
#

if ($action eq 'del') {

    my $classification =
        Bugzilla::Classification::check_classification($class_name);

    if ($classification->id == 1) {
        ThrowUserError("classification_not_deletable");
    }

    if ($classification->product_count()) {
        ThrowUserError("classification_has_products");
    }

    $vars->{'description'} = $classification->description;
    $vars->{'classification'} = $classification->name;

    LoadTemplate($action);
}

#
# action='delete' -> really delete the classification
#

if ($action eq 'delete') {

    my $classification =
        Bugzilla::Classification::check_classification($class_name);

    if ($classification->id == 1) {
        ThrowUserError("classification_not_deletable");
    }

    # lock the tables before we start to change everything:
    $dbh->bz_lock_tables('classifications WRITE', 'products WRITE');

    # delete
    $dbh->do("DELETE FROM classifications WHERE id = ?", undef,
             $classification->id);

    # update products just in case
    $dbh->do("UPDATE products SET classification_id = 1
              WHERE classification_id = ?", undef, $classification->id);

    $dbh->bz_unlock_tables();

    unlink "$datadir/versioncache";

    $vars->{'classification'} = $classification->name;

    LoadTemplate($action);
}

#
# action='edit' -> present the edit classifications from
#
# (next action would be 'update')
#

if ($action eq 'edit') {

    my $classification =
        Bugzilla::Classification::check_classification($class_name);

    my @products =
        Bugzilla::Product::get_products_by_classification(
            $classification->id);

    $vars->{'description'} = $classification->description;
    $vars->{'classification'} = $classification->name;
    $vars->{'products'} = \@products;

    LoadTemplate($action);
}

#
# action='update' -> update the classification
#

if ($action eq 'update') {

    $class_name || ThrowUserError("classification_not_specified");

    my $class_old_name = trim($cgi->param('classificationold') || '');
    my $description    = trim($cgi->param('description')       || '');

    my $class_old =
        Bugzilla::Classification::check_classification($class_old_name);

    $dbh->bz_lock_tables('classifications WRITE');

    if ($class_name ne $class_old->name) {

        my $class = new Bugzilla::Classification({name => $class_name});
        if ($class) {
            ThrowUserError("classification_already_exists",
                           { name => $class->name });
        }
        trick_taint($class_name);
        $dbh->do("UPDATE classifications SET name = ? WHERE id = ?",
                 undef, ($class_name, $class_old->id));
        
        $vars->{'updated_classification'} = 1;

        unlink "$datadir/versioncache";
    }

    if ($description ne $class_old->description) {
        trick_taint($description);
        $dbh->do("UPDATE classifications SET description = ?
                  WHERE id = ?", undef,
                 ($description, $class_old->id));

        $vars->{'updated_description'} = 1;

        unlink "$datadir/versioncache";
    }

    $dbh->bz_unlock_tables();

    LoadTemplate($action);
}

#
# action='reclassify' -> reclassify products for the classification
#

if ($action eq 'reclassify') {

    my $classification =
        Bugzilla::Classification::check_classification($class_name);
   
    $vars->{'description'} = $classification->description;

    my $sth = $dbh->prepare("UPDATE products SET classification_id = ?
                             WHERE name = ?");

    if (defined $cgi->param('add_products')) {
        if (defined $cgi->param('prodlist')) {
            foreach my $prod ($cgi->param("prodlist")) {
                trick_taint($prod);
                $sth->execute($classification->id, $prod);
            }
        }
    } elsif (defined $cgi->param('remove_products')) {
        if (defined $cgi->param('myprodlist')) {
            foreach my $prod ($cgi->param("myprodlist")) {
                trick_taint($prod);
                $sth->execute(1,$prod);
            }
        }
    }

    my @selected_products = ();
    my @unselected_products = ();

    my @products = Bugzilla::Product::get_all_products();

    foreach my $product (@products) {
        if ($product->classification_id == $classification->id) {
            push @selected_products, $product;
        } else {
            push @unselected_products, $product;
        }
    }
    
    $vars->{'selected_products'}   = \@selected_products;
    $vars->{'unselected_products'} = \@unselected_products;
    $vars->{'classification'} = $classification->name;

    LoadTemplate($action);
}

#
# No valid action found
#

ThrowCodeError("action_unrecognized", {action => $action});
