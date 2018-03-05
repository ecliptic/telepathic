open Client;

/** An interface for usage from plain JS */
type t = {
  .
  "makeName": [@bs] (unit => string),
  "getName": [@bs] (unit => Js.Nullable.t(string)),
  "updateName": [@bs] (string => unit),
  "getOrCreateUserName": [@bs] (unit => string),
  "sendMessage": [@bs] (string => unit)
};

type input = {
  .
  "linkId": string,
  "url": Js.Nullable.t(string),
  "onMessage": Js.Nullable.t((clientMessage => unit))
};

/**
 * Make a client and return the plain JS interface
 */
let make: [@bs] (input => t) =
  [@bs]
  (
    (options) => {
      let toOption = Js.Nullable.toOption;
      let callback =
        try (toOption(options##onMessage) |> Js.Option.getExn) {
        | _exn => Js.Exn.raiseError("An 'onMessage' callback is required.")
        };
      let client =
        make(~url=?options##url |> toOption, ~linkId=options##linkId, ~onMessage=callback, ());
      {
        "makeName": [@bs] (() => makeName()),
        "getName": [@bs] (() => getName() |> Js.Nullable.fromOption),
        "updateName": [@bs] ((name) => updateName(~name)),
        "getOrCreateUserName": [@bs] (() => getOrCreateUserName()),
        "sendMessage": [@bs] ((text) => client |> sendMessage(~linkId=client.linkId, ~text))
      }
    }
  );

/** An alias for more semantic usage in Plain JS */
let default = {"start": make};
