# -*- Mode: perl; indent-tabs-mode: nil -*-
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
# The Original Code is Litmus.
#
# The Initial Developer of the Original Code is Netscape Communications
# Corporation. Portions created by Netscape are
# Copyright (C) 1998 Netscape Communications Corporation. All
# Rights Reserved.
#
# Contributor(s): Zach Lipton <zach@zachlipton.com>

package Litmus::DB::Test;

use strict;
use base 'Litmus::DBI';

use Litmus::DB::Testresult;
use Memoize;
use Litmus::Error;

Litmus::DB::Test->table('tests');

Litmus::DB::Test->columns(Primary => qw/testid/);
Litmus::DB::Test->columns(Essential => qw/testid subgroup summary status communityenabled format/); 
Litmus::DB::Test->columns(All => qw/t1 t2 t3 s1 s2 i1 i2/);

Litmus::DB::Test->has_a(subgroup => "Litmus::DB::Subgroup");
Litmus::DB::Test->has_a(status => "Litmus::DB::Status");
Litmus::DB::Test->has_a("format" => "Litmus::DB::Format");

Litmus::DB::Test->has_many(testresults => "Litmus::DB::Testresult", {order_by => 'timestamp DESC'});

# we override Class::DBI's find_column() so that when we refer to 
# formatted names like steps and expected results, we use the actual
# database columns t1 and t2. 
memoize('find_column'); # we use this _a lot_
sub find_column {
    my $self = shift;
    my $want = shift; 

    my $col;
    if (ref $self) {
        $want =~ s/^.*::(\w+)$/$1/;
        $col = $self->format()->getColumnMapping($want);
    }
    
    if ($col) {
        return $self->SUPER::find_column($col);
    } else {
        # didn't find it, so we fall back on the normal 
        # find_column:
        $self->SUPER::find_column($want);
    }
}

# we need to have accessors that correspond to 
# the "virtual columns" created by our format. sounds like a job for 
# autoload...
sub AUTOLOAD {
    my $self = shift;
    my @args = @_;
    my $name = our $AUTOLOAD;
    
    my $col = $self->find_column($name);
    
    if (!$col) {
        internalError("tried to call Litmus::DB::Test method $name which does not exist ");
    }
    
    return $self->$col(@args);
}

# does the test have at least one recent result?
# optionally, just check for a particular platform.
memoize('isrecent');
sub isrecent {
    my $self = shift;
    my $platform = shift;
    
    my %restrictor;
    if ($platform) { $restrictor{platform} = $platform }
    
    my @results = $self->testresults(%restrictor);
    foreach my $curresult (@results) {
        if ($curresult->isrecent()) {
            return 1;
        }
    }
    return 0;
}


# Right now, a test is considered completed as long as it has at least 
# one recent result for that platform. In the future, we would want to 
# use some sort of more advanced system where we look at what results 
# were found and schedule tests with inconsistant results as incomplete. 
memoize('iscompleted');
sub iscompleted {
    my $self = shift;
    my $platform = shift;
    
    if ($self->isrecent($platform)) {
        return 1;
    } else {
        return 0;
    }
}

# You might think that getting the state of a test for a particular platform 
# would be pretty easy. In reality, it's more of an art then a science, since 
# we get to consider all the test results submitted for a particular test, 
# their age, whether the result is from a trusted user, and other fun things. 
#
# Or in other words: "Heuristics are bug ridden by definition. If they didn't 
# have bugs, then they'd be algorithms."
#
# XXX: Rewrite all this as an SQL query so it doesn't take so long. 
memoize('state');
sub state {
    my $self = shift;
    my $platform = shift;
    
    # XXX: if the test is automated, just return the most recent state
    
    my %statecounts; 
    
    # first get all results for this test for this platform:
    my @results = $self->testresults(platform => $platform);
    foreach my $curresult (@results) {
        if (! $curresult->isrecent($platform)) {
            # only consider recent results
            next; 
        } 
        # we weight the result based on its age and if it is confirmed:
        # first figure out how old the result is as a proportion of the 
        # expiration time for the group using a grannuler definition of a day:
        my $adjustedage;
        if ($curresult->age()->days() < 1.5)  {
            $adjustedage = 1;
        } elsif ($curresult->age()->days() < 2.5) {
            $adjustedage = 1.8;
        } else {
            $adjustedage = $curresult->age()->days();
        }
        
        my $ageproportion = $self->subgroup()->testgroup()->expirationdays()/$adjustedage;
        my $weight = $ageproportion;
        
        # give an additional weighting of 2 points to confirmed results:
        if ($curresult->istrusted()) {
            $weight += 2;
        }
        
        $statecounts{$curresult->result()} += $weight;
    }
    
    # now that we have the weighted counts for each possible state, we 
    # calculate the magic number for this test. In other words, the 
    # result spread that we require in order to have confidence in our 
    # result. If the spread between two states is within the magic 
    # number, we just return 0 and the test should be considered unrun
    # since we have no confidence in our result. 
    my $magicnumber = 2; # ok we don't really calculate it. We should though...
    
    foreach my $outer (keys(%statecounts)) {
        foreach my $inner (keys(%statecounts)) {
            if ($outer eq $inner) {
                next;
            }
            if (abs($statecounts{$inner} - $statecounts{$outer}) < $magicnumber) {
                return 0;
            }
        }
    }
    
    # now we just find the state with the greatest value and return it:
    my $maxkey;
    foreach my $cur (keys(%statecounts)) {
        unless ($maxkey) {$maxkey = $cur}
        if ($statecounts{$cur} > $statecounts{$maxkey}) {
            $maxkey = $cur;
        }
    }
    
    return Litmus::DB::Result->retrieve($maxkey);
}

# calculate the percent of the time this test has been in existance that it
# has had a particular state (default state is the current one)
sub percentinstate {
    my $self = shift;
    my $state = shift || $self->state();
    
    
}

# find the number of recent results for this test
memoize('num_recent_results');
sub num_recent_results {
    my $self = shift;
    
    my $count;
    foreach my $curresult ($self->testresults()) {
        if ($curresult->isrecent()) {
            $count++;
        }
    }
    return $count;
}

# these are just convenience functions since they are pretty common needs 
# and templates would be pretty verbose without them:
sub product {
    my $self = shift;
    return $self->testgroup()->product();
}

sub testgroup {
    my $self = shift;
    return $self->subgroup()->testgroup();
}

1;