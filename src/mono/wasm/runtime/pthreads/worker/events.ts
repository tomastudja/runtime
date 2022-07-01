import MonoWasmThreads from "consts:monoWasmThreads";
import type { pthread_ptr } from "../shared";

export const dotnetPthreadCreated = "dotnet:pthread:created" as const;
export const dotnetPthreadAttached = "dotnet:pthread:attached" as const;

/// Events emitted on the current worker when Emscripten uses it to run a pthread.
export interface WorkerThreadEventMap {
    /// Emitted on the current worker when Emscripten first creates a pthread on the current worker.
    /// May be fired multiple times because Emscripten reuses workers to run a new pthread after the current one is finished.
    [dotnetPthreadCreated]: WorkerThreadEvent;
    // Emitted on the current worker when a pthread attaches to Mono.
    // Threads may attach and detach to Mono multiple times during their lifetime.
    [dotnetPthreadAttached]: WorkerThreadEvent;
}

export interface WorkerThreadEvent extends Event {
    readonly pthread_ptr: pthread_ptr;
    readonly portToMain: MessagePort;
}

class WorkerThreadEventImpl extends Event implements WorkerThreadEvent {
    readonly pthread_ptr: pthread_ptr;
    readonly portToMain: MessagePort;
    constructor(type: keyof WorkerThreadEventMap, pthread_ptr: pthread_ptr, portToMain: MessagePort) {
        super(type);
        this.pthread_ptr = pthread_ptr;
        this.portToMain = portToMain;
    }
}


export interface WorkerThreadEventTarget extends EventTarget {
    dispatchEvent(event: WorkerThreadEvent): boolean;

    addEventListener<K extends keyof WorkerThreadEventMap>(type: K, listener: ((this: WorkerThreadEventTarget, ev: WorkerThreadEventMap[K]) => any) | null, options?: boolean | AddEventListenerOptions): void;
    addEventListener(type: string, callback: EventListenerOrEventListenerObject | null, options?: boolean | AddEventListenerOptions): void;
}

export function makeWorkerThreadEvent(type: keyof WorkerThreadEventMap, pthread_ptr: pthread_ptr, port: MessagePort): WorkerThreadEvent {
    // this 'if' helps to tree-shake the WorkerThreadEventImpl class if threads are disabled.
    if (MonoWasmThreads) {
        return new WorkerThreadEventImpl(type, pthread_ptr, port);
    } else {
        throw new Error("threads support disabled");
    }
}
