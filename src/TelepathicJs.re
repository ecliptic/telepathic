open Telepathic;

/** An interface for usage from plain JS */
type t = {
  .
  "makeName": [@bs] (unit => string),
  "getName": [@bs] (unit => Js.Nullable.t(string)),
  "updateName": [@bs] (string => unit),
  "getOrCreateUserName": [@bs] (unit => string),
  "sendMessage": [@bs] (string => unit)
};

/**
 * Make a client and return the plain JS interface
 */
let make: [@bs] ({. "linkId": string, "url": string} => t) =
  [@bs]
  (
    (options) => {
      open Client;
      let client = make(~linkId=options##linkId, options##url);
      {
        "makeName": [@bs] (() => makeName()),
        "getName": [@bs] (() => getName() |> Js.Nullable.from_opt),
        "updateName": [@bs] ((name) => updateName(~name)),
        "getOrCreateUserName": [@bs] (() => getOrCreateUserName()),
        "sendMessage": [@bs] ((text) => client |> sendMessage(~linkId=client.linkId, ~text))
      }
    }
  );

/** An alias for more semantic usage in Plain JS */
let start = make;
