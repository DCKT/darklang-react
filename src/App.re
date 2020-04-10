module TenorApi = {
  let baseUrl = "https://dck-react.builtwithdark.com";

  module V1 = {
    type response = array(result)
    and result = {media: array(mediaType)}
    and mediaType = {gif: media}
    and media = {url: string};

    let search = (search): Js.Promise.t(response) =>
      Js.Promise.(
        Fetch.fetch(baseUrl ++ "/search?q=" ++ search)
        |> then_(Fetch.Response.json)
        |> then_(data => data->Obj.magic)
      );
  };
};

type request('a) =
  | NotAsked
  | Loading
  | Done(result('a, string));

[@react.component]
let make = () => {
  let (search, setSearch) = React.useState(() => "");
  let (results, setResults) = React.useState(() => NotAsked);
  let debounceRef = React.useRef(None);

  React.useEffect1(
    () => {
      switch (search) {
      | "" => ()
      | search =>
        debounceRef
        ->React.Ref.current
        ->Option.map(debRef => {debRef->Js.Global.clearTimeout})
        ->ignore;

        setResults(_ => Loading);

        let timeout =
          Js.Global.setTimeout(
            () => {
              TenorApi.V1.search(search)
              |> Js.Promise.then_(results => {
                   Js.log(results);
                   setResults(_ => Done(Ok(results)));
                   Js.Promise.resolve();
                 })
              |> Js.Promise.catch(err => {
                   Js.log2("error", err);

                   setResults(_ => Done(Error("error")));
                   Js.Promise.resolve();
                 })
              |> ignore
            },
            300,
          );

        debounceRef->React.Ref.setCurrent(Some(timeout));
      };
      Some(
        () => {
          debounceRef
          ->React.Ref.current
          ->Option.map(debRef => {debRef->Js.Global.clearTimeout})
          ->ignore
        },
      );
    },
    [|search|],
  );

  <div className="p-4 text-center">
    <input
      placeholder="Search a gif..."
      className="border p-3 rounded"
      onChange={event => {
        let value = event->ReactEvent.Form.target##value;

        setSearch(_ => value);
      }}
    />
    <div className="mt-4">
      {switch (results) {
       | NotAsked =>
         <p className=""> "Type something in the search bar"->React.string </p>
       | Loading => <p> "Loading..."->React.string </p>
       | Done(Error(_)) => <p> "An error occured"->React.string </p>
       | Done(Ok(data)) =>
         <div className="flex flex-wrap  max-w-screen-lg mx-auto">
           {data
            ->Array.mapWithIndex((i, result) => {
                result.media
                ->Array.get(0)
                ->Option.mapWithDefault(React.null, media => {
                    <img
                      key={i->string_of_int}
                      src={media.gif.url}
                      className="w-1/3 p-2 mb-4"
                    />
                  })
              })
            ->React.array}
         </div>
       }}
    </div>
  </div>;
};