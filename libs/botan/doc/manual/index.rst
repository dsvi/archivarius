
Getting Started
========================================

If you need to build the library first, start with :doc:`building`.
Some Linux distributions include packages for Botan, so building from
source may not be required on your system.

.. only:: html

   The :ref:`genindex` and :ref:`search` may be useful to get started.

.. only:: html and website

   You can also download this manual as a `PDF <https://botan.randombit.net/manual/botan.pdf>`_.

Books and other references
----------------------------

You should have some knowledge of cryptography *before* trying to use
the library. This is an area where it is very easy to make mistakes,
and where things are often subtle and/or counterintuitive. Obviously
the library tries to provide things at a high level precisely to
minimize the number of ways things can go wrong, but naive use will
almost certainly not result in a secure system.

Especially recommended are:

- *Cryptography Engineering*
  by Niels Ferguson, Bruce Schneier, and Tadayoshi Kohno

- `Security Engineering -- A Guide to Building Dependable Distributed Systems
  <https://www.cl.cam.ac.uk/~rja14/book.html>`_ by Ross Anderson

- `Handbook of Applied Cryptography <http://www.cacr.math.uwaterloo.ca/hac/>`_
  by Alfred J. Menezes, Paul C. Van Oorschot, and Scott A. Vanstone

If you're doing something non-trivial or unique, you might want to at
the very least ask for review/input on a mailing list such as the
`metzdowd <http://www.metzdowd.com/mailman/listinfo/cryptography>`_ or
`randombit <https://lists.randombit.net/mailman/listinfo/cryptography>`_
crypto lists. And (if possible) pay a professional cryptographer or
security company to review your design and code.


.. toctree::
   :maxdepth: 1
   :numbered:
