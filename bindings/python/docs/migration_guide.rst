Migration Guide
===============

This document is intended for users who are migrating from the old version of these bindings.

1. Naming changes
-----------------

To conform with PEP 8 the following renames have been completed.

- `rl_client.live_model` -> :meth:`rl_client.LiveModel`
- `rl_client.ranking_response` -> :meth:`rl_client.RankingResponse`
- `rl_client.configuration` -> :meth:`rl_client.Configuration`
- `rl_client.rl_exception` -> :meth:`rl_client.RLException`

2. Error Callback
-----------------

The error callback no longer needs to be a class that inherits from `rl_client.error_callback`. Now it should simply be a function which has a signature of `void(int, string)`, and can include closure values.

.. code-block:: python

    class my_error_callback(rl_client.error_callback):
        def on_error(self, error_code, error_message):
            print("Background error:")
            print(error_message)

     # ...

    error_callback = my_error_callback()
    client = rl_client.live_model(_, error_callback)

Changes to:

.. code-block:: python

    def on_error(self, error_code, error_message):
        print("Background error:")
        print(error_message)

    # ...

    client = rl_client.live_model(_, on_error)


3. Init
-------

`init` no longer needs to be called.

.. code-block:: python

    client = rl_client.live_model(config)
    client.init()

Changes to:

.. code-block:: python

    client = rl_client.live_model(config)

4. `choose_rank` return value
-----------------------------

`choose_rank` no longer returns a tuple, but now returns a :meth:`rl_client.ranking_response` object that contains the same information as was contained in the tuple.

.. code-block:: python

    model_id, chosen_action_id, actions_probabilities, event_id = model.choose_rank(context)


Changes to:

.. code-block:: python

   response = model.choose_rank(context)
   response.model_id
   response.chosen_action_id
   response.actions_probabilities
   response.event_id
