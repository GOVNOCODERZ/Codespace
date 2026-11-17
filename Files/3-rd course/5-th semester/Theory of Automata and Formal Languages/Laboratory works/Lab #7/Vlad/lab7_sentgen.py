"""Lab 7: a context-free grammar for Russian noun phrases and sentences.

Grammar (context-free, axiom S):
    S  -> NP(nom) VP
    NP -> A NP | N
    VP -> Vt NP(acc) | Vi | Vi PP
    PP -> P NP(case)

Modes:
    plain    - no agreement, every word is taken in its base (nominative) form.
    features - adjectives agree with the noun (gender, case, animacy),
               prepositions govern their case (в/на + acc or prep, у + gen).
"""
import random, sys

# Nouns: gender, animacy and forms by case (singular)
NOUNS = {
    'кот':     {'g': 'm', 'anim': True,  'nom': 'кот',      'acc': 'кота',     'gen': 'кота',    'prep': 'коте'},
    'студент': {'g': 'm', 'anim': True,  'nom': 'студент',  'acc': 'студента', 'gen': 'студента', 'prep': 'студенте'},
    'книга':   {'g': 'f', 'anim': False, 'nom': 'книга',    'acc': 'книгу',    'gen': 'книги',   'prep': 'книге'},
    'город':   {'g': 'm', 'anim': False, 'nom': 'город',    'acc': 'город',    'gen': 'города',  'prep': 'городе'},
    'дом':     {'g': 'm', 'anim': False, 'nom': 'дом',      'acc': 'дом',      'gen': 'дома',    'prep': 'доме'},
}

# Adjectives: forms keyed by (gender, case, animate-or-None)
ADJS = {
    'большой':  {('m','nom',None): 'большой', ('m','acc',True): 'большого', ('m','acc',False): 'большой',
                 ('m','gen',None): 'большого', ('m','prep',None): 'большом',
                 ('f','nom',None): 'большая', ('f','acc',None): 'большую',
                 ('f','gen',None): 'большой', ('f','prep',None): 'большой'},
    'умный':    {('m','nom',None): 'умный', ('m','acc',True): 'умного', ('m','acc',False): 'умный',
                 ('m','gen',None): 'умного', ('m','prep',None): 'умном',
                 ('f','nom',None): 'умная', ('f','acc',None): 'умную',
                 ('f','gen',None): 'умной', ('f','prep',None): 'умной'},
    'красивый': {('m','nom',None): 'красивый', ('m','acc',True): 'красивого', ('m','acc',False): 'красивый',
                 ('m','gen',None): 'красивого', ('m','prep',None): 'красивом',
                 ('f','nom',None): 'красивая', ('f','acc',None): 'красивую',
                 ('f','gen',None): 'красивой', ('f','prep',None): 'красивой'},
}
VT = ['читает', 'видит', 'строит']          # transitive verbs
VI = ['спит', 'живёт']                      # intransitive verbs
PREP = {'в': ['acc', 'prep'], 'на': ['acc', 'prep'], 'у': ['gen']}   # preposition -> cases

class Sym:
    """A grammar symbol: a nonterminal (name, case, noun, k) or a terminal word (text)."""
    def __init__(self, name, case=None, noun=None, adj=None, k=None, word=None):
        self.name, self.case, self.noun, self.adj, self.k, self.word = name, case, noun, adj, k, word
    def show(self):
        return self.word if self.word is not None else self.name

class Generator:
    def __init__(self, mode, rnd):
        self.mode, self.rnd = mode, rnd

    def noun_word(self, noun, case):
        return NOUNS[noun]['nom'] if self.mode == 'plain' else NOUNS[noun][case]

    def adj_word(self, adj, noun, case):
        if self.mode == 'plain':
            return ADJS[adj][('m', 'nom', None)]
        n = NOUNS[noun]
        animate = n['anim'] if (n['g'] == 'm' and case == 'acc') else None
        return ADJS[adj][(n['g'], case, animate)]

    def expand(self, s):
        """Apply one rule to nonterminal s. Returns (rule text, replacement list)."""
        r = self.rnd
        if s.name == 'S':
            return 'S → NP VP', [Sym('NP', case='nom'), Sym('VP')]
        if s.name == 'NP':
            if s.noun is None:                       # choose head noun and adjective count first
                pool = list(NOUNS)
                if self.mode == 'features' and s.case == 'nom':
                    pool = [n for n in NOUNS if NOUNS[n]['anim']]   # subjects must be animate
                s.noun = r.choice(pool)
                s.k = r.randrange(3) if self.mode == 'features' else r.randrange(2)
            if s.k == 0:
                return 'NP → N', [Sym('N', case=s.case, noun=s.noun)]
            s.adj = r.choice(list(ADJS))
            return 'NP → A NP', [Sym('A', case=s.case, noun=s.noun, adj=s.adj),
                                 Sym('NP', case=s.case, noun=s.noun, k=s.k - 1)]
        if s.name == 'VP':
            opt = r.randrange(3)
            if opt == 0: return 'VP → Vt NP', [Sym('Vt'), Sym('NP', case='acc')]
            if opt == 1: return 'VP → Vi', [Sym('Vi')]
            return 'VP → Vi PP', [Sym('Vi'), Sym('PP')]
        if s.name == 'PP':
            p = r.choice(list(PREP)); case = r.choice(PREP[p])
            return 'PP → P NP', [Sym('P', word=p), Sym('NP', case=case)]
        if s.name in ('Vt', 'Vi'):
            w = r.choice(VT if s.name == 'Vt' else VI)
            return f'{s.name} → {w}', [Sym(s.name, word=w)]
        if s.name == 'N':
            w = self.noun_word(s.noun, s.case)
            return f'N → {w}', [Sym('N', word=w)]
        if s.name == 'A':
            w = self.adj_word(s.adj, s.noun, s.case)
            return f'A → {w}', [Sym('A', word=w)]
        raise ValueError(s.name)

    def derive(self):
        """Leftmost derivation from S. Returns (list of (rule, sentential form), sentence)."""
        form = [Sym('S')]
        steps = []
        while any(x.word is None for x in form):
            i = next(j for j, x in enumerate(form) if x.word is None)   # leftmost nonterminal
            rule, repl = self.expand(form[i])
            form = form[:i] + repl + form[i + 1:]
            steps.append((rule, ' '.join(x.show() for x in form)))
        return steps, ' '.join(x.word for x in form)

def capitalize(s):
    return s[0].upper() + s[1:] + '.'

if __name__ == '__main__':
    mode = sys.argv[1] if len(sys.argv) > 1 else 'plain'
    seed = int(sys.argv[2]) if len(sys.argv) > 2 else 7
    n = int(sys.argv[3]) if len(sys.argv) > 3 else 5
    show = len(sys.argv) > 4 and sys.argv[4] == 'trace'
    rnd = random.Random(seed)
    g = Generator(mode, rnd)
    for i in range(n):
        steps, sentence = g.derive()
        print('Sentence %d: %s' % (i + 1, capitalize(sentence)))
        if show:
            print('  axiom: S')
            for rule, form in steps:
                print('  %-14s => %s' % (rule, form))
