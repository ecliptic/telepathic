open Js.Promise;

/**
 * Checks to see if the expected value matches the actual, raising an exception otherwise
 */
let expectMatches = (~message: string, ~expected: 'a, ~actual: 'a) =>
  if (actual !== expected) {
    Js.Exn.raiseError(
      "Failed: " ++ message ++ "\nExpected: " ++ expected ++ "\nActual: " ++ actual
    )
  };

/**
 * Safely converts an object to json by stringifying it and parsing the results.
 */
let objToJson = (obj: Js.t('a)) : Js.Json.t => {
  let str = Js.Json.stringifyAny(obj);
  /* parseExn should be safe because the string came from stringifyAny */
  switch str {
  | Some(str) => str |> Js.Json.parseExn
  | None => Js.Json.null
  }
};

/** Create a new error instance */ [@bs.new] external makeError : string => exn = "Error";

/**
 * Magically convert a Promise error to a standard JS exception
 */
external toJsExn : Js.Promise.error => Js.Exn.t =
  "%identity";

/**
 * Reject a promise with an error using the given message
 */
let rejectWith = (message) => reject(makeError(message));

let cancelTimeout = (delay: int, message: string) =>
  make(
    (~resolve as _, ~reject) =>
      delay |> Js.Global.setTimeout(() => [@bs] reject(makeError(message))) |> ignore
  );
